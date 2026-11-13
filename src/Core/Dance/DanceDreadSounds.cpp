#include "include/Core/Dance/DanceDreadParts.hpp"

#include "UnityEngine/HideFlags.hpp"

#include "assets.hpp"
#include "main.hpp"

#include <array>
#include <cstring>
#include <functional>
#include <random>
#include <span>

using UnityEngine::AudioClip;

namespace DanceDread::Sounds {
    static constexpr int SampleRate = 44100;
    static constexpr float Pi = (float)M_PI;

    static constexpr float VowelFormants[5][3] = {
        {730, 1090, 2440}, {530, 1840, 2480}, {270, 2290, 3010}, {570, 840, 2410}, {300, 870, 2240}
    };

    static safe_ptr<AudioClip*> heartbeat;
    static safe_ptr<AudioClip*> sting;
    static safe_ptr<AudioClip*> relief;

    #pragma region Clips

    static AudioClip* FromSamples(std::string const& name, std::vector<float> const& samples, int rate) {
        if (samples.empty()) return nullptr;
        auto clip = AudioClip::Create(name, (int)samples.size(), 1, rate, false);
        ArrayW<float> data(samples.size());
        std::copy(samples.begin(), samples.end(), data.begin());
        clip->SetData(data, 0);
        clip->hideFlags = UnityEngine::HideFlags::DontUnloadUnusedAsset;
        return clip.ptr();
    }

    static AudioClip* Synthesize(std::string const& name, float seconds, std::function<float(float)> const& sample) {
        std::vector<float> samples((size_t)std::ceil(seconds * SampleRate));
        for (size_t i = 0; i < samples.size(); i++) {
            samples[i] = std::clamp(sample((float)i / SampleRate), -1.0f, 1.0f);
        }
        return FromSamples(name, samples, SampleRate);
    }

    static AudioClip* LoadWav(std::string const& name, std::span<uint8_t const> bytes) {
        if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) return nullptr;
        int rate = 0, channels = 0, bits = 0;
        size_t position = 12;
        while (position + 8 <= bytes.size()) {
            auto id = bytes.data() + position;
            size_t size = ReadValue<uint32_t>(id + 4);
            auto chunk = id + 8;
            if (position + 8 + size > bytes.size()) size = bytes.size() - position - 8;
            if (std::memcmp(id, "fmt ", 4) == 0 && size >= 16) {
                channels = ReadValue<uint16_t>(chunk + 2);
                rate = (int)ReadValue<uint32_t>(chunk + 4);
                bits = ReadValue<uint16_t>(chunk + 14);
            } else if (std::memcmp(id, "data", 4) == 0) {
                if (bits != 16 || channels != 1 || rate <= 0) return nullptr;
                std::vector<float> samples(size / 2);
                for (size_t i = 0; i < samples.size(); i++) {
                    samples[i] = ReadValue<int16_t>(chunk + i * 2) / 32768.0f;
                }
                return FromSamples(name, samples, rate);
            }
            position += 8 + size + (size & 1);
        }
        return nullptr;
    }

    #pragma endregion

    #pragma region Synthesis

    struct Biquad {
        float b0 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

        static Biquad BandPass(float frequency, float q) {
            float w0 = 2 * Pi * frequency / SampleRate;
            float alpha = std::sin(w0) / (2 * q);
            float a0 = 1 + alpha;
            Biquad filter;
            filter.b0 = alpha / a0;
            filter.b2 = -alpha / a0;
            filter.a1 = -2 * std::cos(w0) / a0;
            filter.a2 = (1 - alpha) / a0;
            return filter;
        }

        float Process(float x) {
            float y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }
    };

    static float Thump(float t, float start, float length, float fromHz, float toHz) {
        float x = t - start;
        if (x < 0 || x > length) return 0;
        float progress = x / length;
        float phase = 2 * Pi * (fromHz * x + (toHz - fromHz) * x * progress * 0.5f);
        float envelope = Clamp01(x * 200) * std::pow(1 - progress, 2.0f);
        return std::sin(phase) * envelope;
    }

    static float Bell(float t, float start, float hz) {
        float x = t - start;
        if (x < 0) return 0;
        float envelope = Clamp01(x * 80) * std::exp(-x * 3.5f);
        return (std::sin(2 * Pi * hz * x) + 0.3f * std::sin(4 * Pi * hz * x)) * envelope;
    }

    static AudioClip* CreateWhisper(int seed) {
        std::mt19937 random(seed);
        auto next = [&random] { return std::uniform_real_distribution<float>(0, 1)(random); };
        int syllables = 5 + (int)(next() * 4);
        std::vector<float> data((size_t)(SampleRate * (0.25f * syllables + 0.3f)));
        size_t position = 0;
        for (int s = 0; s < syllables && position < data.size(); s++) {
            int length = (int)(SampleRate * (0.16f + 0.12f * next()));
            auto& formants = VowelFormants[std::min(4, (int)(next() * 5))];
            int hiss = next() < 0.45f ? (int)(SampleRate * 0.07f) : 0;
            Biquad filters[3];
            for (int f = 0; f < 3; f++) filters[f] = Biquad::BandPass(formants[f], formants[f] / 120);
            auto hissFilter = Biquad::BandPass(6000, 1.2f);
            for (int i = 0; i < hiss + length && position < data.size(); i++, position++) {
                float noise = next() * 2 - 1;
                float sample;
                if (i < hiss) {
                    float x = (float)i / hiss;
                    sample = 0.5f * hissFilter.Process(noise) * std::sin(Pi * x);
                } else {
                    float x = (float)(i - hiss) / length;
                    float envelope = Clamp01(x * 8) * std::pow(1 - x, 1.5f);
                    sample = (filters[0].Process(noise) + 0.6f * filters[1].Process(noise) + 0.3f * filters[2].Process(noise)) * envelope;
                }
                data[position] = sample;
            }
            position += (size_t)(SampleRate * 0.03f * next());
        }
        float peak = 0.0001f;
        for (float v : data) peak = std::max(peak, std::abs(v));
        for (float& v : data) v *= 0.8f / peak;
        return FromSamples("DanceDreadWhisper" + std::to_string(seed), data, SampleRate);
    }

    #pragma endregion

    #pragma region Voices

    struct VoiceLine {
        std::string_view name;
        uint8_t* (*data)();
        size_t (*length)();
    };

    #define DANCE_VOICE(name, asset) VoiceLine{name, &asset::getData, &asset::getLength}

    static constexpr VoiceLine VoiceLines[] = {
        DANCE_VOICE("dance", voice_dance_wav),
        DANCE_VOICE("dance-or-die", voice_dance_or_die_wav),
        DANCE_VOICE("dont-stop", voice_dont_stop_wav),
        DANCE_VOICE("keep-moving", voice_keep_moving_wav),
        DANCE_VOICE("not-yet", voice_not_yet_wav),
        DANCE_VOICE("we-see-you", voice_we_see_you_wav),
        DANCE_VOICE("yours", voice_yours_wav),
    };

    #undef DANCE_VOICE

    #pragma endregion

    static constexpr int WhisperSeeds[] = {11, 23, 37, 53};

    static std::array<safe_ptr<AudioClip*>, std::size(VoiceLines)> voices;
    static std::array<safe_ptr<AudioClip*>, std::size(WhisperSeeds)> whispers;

    static AudioClip* Whisper(int index) {
        if (!whispers[index]) whispers[index] = CreateWhisper(WhisperSeeds[index]);
        return Alive(whispers[index]);
    }

    void Preload() {
        Heartbeat();
        Sting();
        Relief();
        for (int i = 0; i < (int)whispers.size(); i++) Whisper(i);
        for (auto const& line : VoiceLines) Voice(line.name);
    }

    AudioClip* Heartbeat() {
        if (!heartbeat) {
            heartbeat = Synthesize("DanceDreadHeartbeat", 0.45f, [](float t) {
                return Thump(t, 0, 0.11f, 68, 42) + 0.75f * Thump(t, 0.18f, 0.1f, 80, 50);
            });
        }
        return Alive(heartbeat);
    }

    AudioClip* Sting() {
        if (!sting) {
            sting = Synthesize("DanceDreadSting", 2.2f, [](float t) {
                float boom = Thump(t, 0, 1.4f, 58, 28);
                float decay = std::exp(-t * 1.6f) * Clamp01(t * 30);
                float tremolo = 0.75f + 0.25f * std::sin(2 * Pi * 5.5f * t);
                float cluster = (std::sin(2 * Pi * 207.65f * t) + std::sin(2 * Pi * 220 * t) + 0.5f * std::sin(2 * Pi * 311.1f * t)) / 2.5f;
                return 0.8f * boom + 0.35f * cluster * decay * tremolo;
            });
        }
        return Alive(sting);
    }

    AudioClip* Relief() {
        if (!relief) {
            relief = Synthesize("DanceDreadRelief", 1.4f, [](float t) {
                return 0.3f * Bell(t, 0, 659.25f) + 0.25f * Bell(t, 0.12f, 987.77f);
            });
        }
        return Alive(relief);
    }

    AudioClip* RandomWhisper() {
        return Whisper(RandomRange(0, (int)whispers.size()));
    }

    AudioClip* Voice(std::string_view name) {
        for (size_t i = 0; i < voices.size(); i++) {
            auto const& line = VoiceLines[i];
            if (line.name != name) continue;
            if (!voices[i]) {
                voices[i] = LoadWav("voice-" + std::string(name), std::span<uint8_t const>(line.data(), line.length()));
                if (!voices[i]) BeatLeaderLogger.warn("Dance or Die: can't decode voice-{}.wav", name);
            }
            return Alive(voices[i]);
        }
        return nullptr;
    }
}
