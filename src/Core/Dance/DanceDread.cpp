#include "include/Utils/ArrayUtils.hpp"
#include "include/Core/Dance/DanceDread.hpp"
#include "include/Core/Dance/DanceDreadParts.hpp"
#include "include/Core/Dance/DanceEvent.hpp"
#include "include/Core/CustomModifiers.hpp"
#include "include/Enhancers/MapEnhancer.hpp"

#include "GlobalNamespace/AudioManager.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapCharacteristicExtensions.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BloomFogEnvironmentParams.hpp"
#include "GlobalNamespace/BloomFogSO.hpp"
#include "GlobalNamespace/BpmController.hpp"
#include "GlobalNamespace/EnvironmentEffectsFilterPreset.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "BeatSaber/Haptics/HapticFeedbackManager.hpp"
#include "GlobalNamespace/IBpmController.hpp"
#include "GlobalNamespace/IGamePause.hpp"
#include "GlobalNamespace/MainAudioEffects.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "BeatSaber/Haptics/HapticPresetSO.hpp"
#include "System/Type.hpp"
#include "UnityEngine/Audio/AudioMixer.hpp"
#include "UnityEngine/AudioLowPassFilter.hpp"
#include "UnityEngine/AudioRolloffMode.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/ScriptableObject.hpp"
#include "UnityEngine/XR/XRNode.hpp"
#include "Zenject/DiContainer.hpp"

#include "main.hpp"

#include <array>
#include <cmath>

using namespace GlobalNamespace;
using BeatSaber::Haptics::HapticFeedbackManager;
using BeatSaber::Haptics::HapticPresetSO;
using UnityEngine::Vector3;

namespace DanceDread {
    float TrailDroop = 0;
    float NoteGray = 0;
    float NoteDim = 0;

    static bool haunting = false;
    static bool warning = false;
    static std::optional<Summary> lastSummary;

    bool IsHaunting() {
        return haunting;
    }

    bool IsWarning() {
        return warning;
    }

    std::optional<Summary> const& LastSummary() {
        return lastSummary;
    }

    #pragma region Level

    static constexpr std::string_view CustomLevelPrefix = "custom_level_";

    static bool eventLevel = false;
    static bool flickerAllowed = true;
    static std::string levelHash;
    static std::string levelDifficulty;
    static std::string levelMode;
    static safe_ptr<Zenject::DiContainer*> levelContainer;

    static safe_ptr<AudioTimeSyncController*> timeSyncController;
    static safe_ptr<GameEnergyCounter*> energyCounter;
    static safe_ptr<BpmController*> bpmController;
    static safe_ptr<GamePause*> gamePause;
    static safe_ptr<MainAudioEffects*> mainAudioEffects;
    static safe_ptr<AudioManager*> audioManager;
    static safe_ptr<HapticFeedbackManager*> hapticFeedbackManager;

    // Zenject's TryResolve still throws when there are several bindings
    static System::Object* TryResolve(Zenject::DiContainer* container, System::Type* type) {
        try {
            return container->TryResolve(type);
        } catch (std::exception const& e) {
            BeatLeaderLogger.warn("Dance or Die: can't resolve {}: {}", (std::string)type->get_Name(), e.what());
            return nullptr;
        }
    }

    template <typename T, typename Bound = T>
    static T* Resolve(Zenject::DiContainer* container) {
        return i2c::try_cast<T*>(TryResolve(container, i2c::cs_type_of<Bound*>()));
    }

    template <typename T>
    static T* FindActive() {
        auto found = UnityEngine::Object::FindObjectsOfType(i2c::cs_type_of<T*>());
        return found.size() > 0 ? reinterpret_cast<T*>(found[0].unsafe_ptr()) : nullptr;
    }

    template <typename T>
    static T* CreateScriptable() {
        return reinterpret_cast<T*>(UnityEngine::ScriptableObject::CreateInstance(i2c::cs_type_of<T*>()).unsafe_ptr());
    }

    #pragma endregion

    #pragma region Constants

    static constexpr float EaseSeconds = 1;
    static constexpr float LightsOut = 0.3f;
    static constexpr float TheDark = 0.55f;
    static constexpr float NearDeathEnergy = 0.12f;

    static constexpr float MaxCutoff = 22000;
    static constexpr float MinCutoff = 1500;

    static constexpr float DreadFogThickness = 6;
    static constexpr float DreadFogMinAttenuation = 0.02f;

    static constexpr float HeartbeatSeconds = 0.8f;
    static constexpr float WhisperDistance = 1.8f;
    static constexpr int WhisperSources = 2;

    static constexpr std::array<std::string_view, 3> StopLines = {"dont-stop", "we-see-you", "keep-moving"};

    #pragma endregion

    struct State {
        float dread = 0;
        float shown = 0;
        float lastLiveTime = -1;
        float lastSongTime = 0;
        int stops = 0;
        bool wasStill = false;
        float stillSeconds = 0;

        float cutoff = MaxCutoff;
        float appliedCutoff = -1;
        bool lowPassTouched = false;
        bool basePitchKnown = false;
        float basePitch = 1;
        float appliedPitch = 1;

        float nextFlicker = 3;
        float flickerTimer = 0;
        float flickerOff = 0;

        bool fogUnavailable = false;
        bool fogApplied = false;
        float savedTransition = 0;
        float appliedTransition = -1;

        float appliedVignette = -1;

        float nextBeat = NAN;
        bool wasHaunted = false;
        std::array<float, WhisperSources> whisperAngles{};
        std::array<float, WhisperSources> whisperSpeeds{};
        float whisperTimer = 0;
        float nextWhisper = 3;
        int whisperCount = 0;

        float voiceCooldown = 0;
        int voiceStops = 0;
        bool wasWarning = false;
        bool spokeInTheDark = false;
        int lastStopLine = -1;
    };

    static State s;

    static safe_ptr<BloomFogSO*> bloomFog;
    static safe_ptr<BloomFogEnvironmentParams*> dreadFog;
    static safe_ptr<BloomFogEnvironmentParams*> savedTransitionFog;
    static safe_ptr<UnityEngine::GameObject*> vignette;
    static safe_ptr<UnityEngine::UI::Image*> vignetteImage;
    static safe_ptr<UnityEngine::AudioSource*> audioSource;
    static safe_ptr<UnityEngine::AudioSource*> voiceSource;
    static std::array<safe_ptr<UnityEngine::AudioSource*>, WhisperSources> whisperSources;
    static safe_ptr<HapticPresetSO*> hapticPreset;

    #pragma region Dread

    static float Stage(float start, float end = 1) {
        return SmoothStep01(InverseLerp(start, end, s.shown));
    }

    static float NearDeath() {
        auto counter = Alive(energyCounter);
        if (!counter || Stage(LightsOut) <= 0) return 0;
        return InverseLerp(NearDeathEnergy, 0, counter->_energy_k__BackingField);
    }

    static void UpdateDread(Dance::DanceLive const& live) {
        if (live.SongTime == s.lastLiveTime) return;
        float step = s.lastLiveTime < 0 ? 0 : live.SongTime - s.lastLiveTime;
        s.lastLiveTime = live.SongTime;
        if (!live.Ready) {
            s.dread = 0;
            return;
        }
        if (live.CountedFraction < Dance::DanceScorer::LiveMinCounted) return;
        bool still = live.Level < DreadLine;
        if (still) {
            s.dread += 0.12f + 0.12f * (float)((DreadLine - live.Level) / DreadLine);
            s.stillSeconds += step;
            lastSummary->LongestStillSeconds = std::max(lastSummary->LongestStillSeconds, s.stillSeconds);
            if (!s.wasStill) s.stops++;
        } else {
            s.dread -= 0.15f;
            s.stillSeconds = 0;
        }
        s.wasStill = still;
        s.dread = Clamp01(s.dread);
    }

    #pragma endregion

    #pragma region Music

    static void UpdateLowPass(float dt) {
        auto effects = Alive(mainAudioEffects);
        if (!effects) return;
        bool gameUsesLowPass = effects->enabled;
        if (gameUsesLowPass) {
            s.appliedCutoff = -1;
            return;
        }
        auto filter = effects->_audioLowPassFilter;
        if (!filter) return;
        float amount = 0.45f * Stage(0.03f, 0.35f) + 0.55f * Stage(0.35f);
        float target = MaxCutoff * std::pow(MinCutoff / MaxCutoff, amount);
        s.cutoff = std::exp(Lerp(std::log(s.cutoff), std::log(target), 1 - std::exp(-dt / 0.3f)));
        bool on = s.cutoff < MaxCutoff * 0.95f;
        float cutoff = on ? s.cutoff : MaxCutoff;
        if (std::abs(cutoff - s.appliedCutoff) < 1) return;
        s.appliedCutoff = cutoff;
        s.lowPassTouched = true;
        filter->enabled = on;
        filter->cutoffFrequency = cutoff;
    }

    static void RestoreLowPass() {
        auto effects = Alive(mainAudioEffects);
        if (!s.lowPassTouched || !effects || effects->enabled) return;
        auto filter = effects->_audioLowPassFilter;
        if (!filter) return;
        filter->cutoffFrequency = MaxCutoff;
        filter->enabled = false;
    }

    static void ReadBasePitch() {
        auto manager = Alive(audioManager);
        if (!manager || !manager->_audioMixer) return;
        float pitch = 1;
        if (manager->_audioMixer->GetFloat("MusicPitch", by_ref<float>(pitch)) && pitch > 0) {
            s.basePitch = pitch;
            s.basePitchKnown = true;
        } else {
            BeatLeaderLogger.warn("Dance or Die: can't read the music pitch, leaving it alone");
        }
    }

    static void UpdatePitch() {
        auto manager = Alive(audioManager);
        if (!manager || !s.basePitchKnown) return;
        float pitch = std::pow(2.0f, -2 * Stage(0.8f) / 12);
        if (std::abs(pitch - s.appliedPitch) < 0.001f) return;
        s.appliedPitch = pitch;
        manager->musicPitch = s.basePitch * pitch;
    }

    static void RestorePitch() {
        auto manager = Alive(audioManager);
        if (!manager || !s.basePitchKnown || s.appliedPitch == 1) return;
        manager->musicPitch = s.basePitch;
    }

    #pragma endregion

    #pragma region Lights

    static void UpdateLights(float dt) {
        float amount = Stage(0.2f);
        float dim = 0.8f * amount;
        float dark = Stage(TheDark);
        if (flickerAllowed && dark > 0) {
            s.flickerTimer += dt;
            if (s.flickerOff > 0) {
                s.flickerOff -= dt;
                dim = 1 - (1 - dim) * 0.25f;
            } else if (s.flickerTimer >= s.nextFlicker) {
                s.flickerTimer = 0;
                s.flickerOff = RandomRange(0.12f, 0.25f);
                s.nextFlicker = RandomValue() < 0.4f ? 0.35f : Lerp(6, 2, dark) * RandomRange(0.7f, 1.3f);
            }
        }
        Lights::Apply(dim, 0.6f * amount, 0.5f * Stage(0.03f));
    }

    #pragma endregion

    #pragma region Fog

    static void UpdateFog(float amount) {
        auto fog = Alive(bloomFog);
        if (!fog || s.fogUnavailable) return;
        if (!s.fogApplied) {
            if (amount <= 0) return;
            auto current = fog->defaultForParams;
            if (!current) {
                s.fogUnavailable = true;
                return;
            }
            savedTransitionFog = fog->transitionFogParams.unsafe_ptr();
            s.savedTransition = fog->transition;
            auto params = CreateScriptable<BloomFogEnvironmentParams>();
            params->attenuation = std::max(current->attenuation * DreadFogThickness, DreadFogMinAttenuation);
            params->offset = current->offset * 0.3f;
            params->heightFogStartY = current->heightFogStartY;
            params->heightFogHeight = current->heightFogHeight;
            params->autoExposureLimit = current->autoExposureLimit;
            params->legacyAutoExposure = current->legacyAutoExposure;
            params->noteSpawnIntensity = current->noteSpawnIntensity;
            dreadFog = params;
            fog->transitionFogParams = params;
            s.fogApplied = true;
        }
        if (std::abs(amount - s.appliedTransition) < 0.002f) return;
        s.appliedTransition = amount;
        fog->transition = amount;
    }

    static void RestoreFog() {
        auto fog = Alive(bloomFog);
        if (fog && s.fogApplied) {
            fog->transitionFogParams = Alive(savedTransitionFog);
            fog->transition = s.savedTransition;
        }
        Destroy(dreadFog);
        savedTransitionFog = nullptr;
        bloomFog = nullptr;
    }

    #pragma endregion

    #pragma region Vignette

    static UnityEngine::Sprite* VignetteSprite() {
        static safe_ptr<UnityEngine::Sprite*> sprite;
        if (!sprite) {
            sprite = CreateAlphaSprite(256, 256, {0.5f, 0.5f}, [](float u, float v) {
                return SmoothStep01(InverseLerp(0.33f, 0.75f, CenterDistance(u, v)));
            });
        }
        return Alive(sprite);
    }

    static void CreateVignette() {
        auto head = Head();
        if (!head) return;
        auto [root, image] = CreateImage("DanceDreadVignette", VignetteSprite(), {3, 3}, NoGlowMaterial());
        root->GetComponent<UnityEngine::Canvas*>()->sortingOrder = 32000;
        root->transform->SetParent(head, false);
        root->transform->localPosition = {0, 0, 0.4f};
        vignette = root;
        vignetteImage = image;
        s.appliedVignette = -1;
    }

    static void UpdateVignette() {
        float alpha = 0.35f * Stage(0.02f, 0.3f) + 0.57f * Stage(TheDark);
        if (!vignette) {
            if (alpha <= 0) return;
            CreateVignette();
        }
        auto image = Alive(vignetteImage);
        if (!image || std::abs(alpha - s.appliedVignette) < 0.002f) return;
        s.appliedVignette = alpha;
        image->color = {0, 0, 0, alpha};
    }

    #pragma endregion

    #pragma region Sounds

    static UnityEngine::AudioSource* CreateSpatialSource(std::string_view name, float minDistance) {
        auto source = UnityEngine::GameObject::New_ctor(std::string(name))->AddComponent<UnityEngine::AudioSource*>();
        source->playOnAwake = false;
        source->spatialBlend = 1;
        source->rolloffMode = UnityEngine::AudioRolloffMode::Linear;
        source->minDistance = minDistance;
        source->maxDistance = 20;
        source->dopplerLevel = 0;
        return source;
    }

    static void CreateAudio() {
        auto source = UnityEngine::GameObject::New_ctor("DanceDreadAudio")->AddComponent<UnityEngine::AudioSource*>();
        source->spatialBlend = 0;
        source->playOnAwake = false;
        audioSource = source;

        auto preset = CreateScriptable<HapticPresetSO>();
        preset->_duration = 0.08f;
        preset->_frequency = 0.2f;
        preset->_useAdvancedHapticsOnSupportedPlatforms = false;
        hapticPreset = preset;
    }

    static void Play(UnityEngine::AudioClip* clip, float volume) {
        if (auto source = Alive(audioSource)) source->PlayOneShot(clip, volume);
    }

    static void UpdateHeartbeat(float songTime) {
        if (songTime <= 0) return;
        auto bpm = bpmController ? bpmController->_currentBpm : 0;
        float beat = bpm > 0 ? 60 / bpm : 0;
        float interval = beat > 0.05f ? beat * std::max(1, (int)std::round(HeartbeatSeconds / beat)) : HeartbeatSeconds;
        float following = (std::floor(songTime / interval) + 1) * interval;
        if (std::isnan(s.nextBeat)) s.nextBeat = following;
        if (songTime < s.nextBeat) return;
        s.nextBeat = following;

        float volume = 0.4f * Stage(0.02f, 0.25f) + 0.5f * Stage(0.25f);
        if (volume <= 0.01f) return;
        Play(Sounds::Heartbeat(), 0.9f * volume);

        float haptics = Stage(LightsOut);
        auto manager = Alive(hapticFeedbackManager);
        auto preset = Alive(hapticPreset);
        if (!manager || !preset || haptics <= 0.01f) return;
        preset->_strength = 0.7f * haptics;
        manager->PlayHapticFeedback(UnityEngine::XR::XRNode::LeftHand, preset);
        manager->PlayHapticFeedback(UnityEngine::XR::XRNode::RightHand, preset);
    }

    static Vector3 AroundHead(Vector3 head, float angle) {
        return Add(head, Scale(AroundY(angle), WhisperDistance));
    }

    static void UpdateWhispers(float dt) {
        auto head = HeadPosition();
        for (int i = 0; i < WhisperSources; i++) {
            auto source = Alive(whisperSources[i]);
            if (!source || !source->isPlaying) continue;
            s.whisperAngles[i] += s.whisperSpeeds[i] * dt;
            source->transform->position = AroundHead(head, s.whisperAngles[i]);
        }
        float early = Stage(0.15f, 0.4f);
        if (early <= 0) return;
        s.whisperTimer += dt;
        if (s.whisperTimer < s.nextWhisper) return;
        float dark = Stage(TheDark);
        s.whisperTimer = 0;
        s.nextWhisper = RandomRange(4.0f, 9.0f) * Lerp(1.6f, 0.8f, dark);
        int index = s.whisperCount++ % WhisperSources;
        if (!whisperSources[index]) whisperSources[index] = CreateSpatialSource("DanceDreadWhisper", 1);
        auto whisper = whisperSources[index].ptr();
        s.whisperAngles[index] = RandomRange(0.0f, 360.0f);
        s.whisperSpeeds[index] = RandomRange(30.0f, 70.0f) * (RandomValue() < 0.5f ? -1 : 1);
        whisper->clip = Sounds::RandomWhisper();
        whisper->volume = 0.25f * early + 0.35f * dark;
        whisper->transform->position = AroundHead(head, s.whisperAngles[index]);
        whisper->Play();
    }

    static bool Say(std::string_view line, bool force = false) {
        if (!force && s.voiceCooldown > 0) return false;
        auto clip = Sounds::Voice(line);
        auto head = Head();
        if (!clip || !head) return false;
        if (!voiceSource) voiceSource = CreateSpatialSource("DanceDreadVoice", 0.5f);
        auto source = voiceSource.ptr();
        Vector3 forward = head->forward;
        Vector3 back = {-forward.x, 0, -forward.z};
        float length = std::sqrt(back.x * back.x + back.z * back.z);
        back = length > 0.0001f ? Scale(back, 1 / length) : Vector3(0, 0, -1);
        float side = RandomValue() < 0.5f ? -0.25f : 0.25f;
        Vector3 offset = {back.z * side, 0, -back.x * side};
        source->transform->position = Add(Add(GetPosition(head), Scale(back, 0.5f)), offset);
        source->clip = clip;
        source->Play();
        s.voiceCooldown = 8;
        return true;
    }

    static void UpdateVoice(float dt) {
        s.voiceCooldown -= dt;
        if (!s.spokeInTheDark && Stage(TheDark) > 0.3f) s.spokeInTheDark = Say("dance");
        if (warning && !s.wasWarning) Say("dance-or-die", true);
        s.wasWarning = warning;
        if (s.stops == s.voiceStops) return;
        s.voiceStops = s.stops;
        if (Stage(LightsOut) <= 0) return;
        int index = RandomRange(0, (int)StopLines.size());
        if (index == s.lastStopLine) index = (index + 1) % (int)StopLines.size();
        if (Say(StopLines[index])) s.lastStopLine = index;
    }

    static void UpdateRelief() {
        if (s.shown >= 0.5f) {
            s.wasHaunted = true;
        } else if (s.wasHaunted && s.shown < 0.1f) {
            s.wasHaunted = false;
            lastSummary->Escapes++;
            Play(Sounds::Relief(), 0.6f);
            Say("not-yet", true);
        }
    }

    static void DestroySource(safe_ptr<UnityEngine::AudioSource*>& source) {
        if (auto alive = Alive(source)) UnityEngine::Object::Destroy(alive->gameObject);
        source = nullptr;
    }

    #pragma endregion

    #pragma region Death

    static void HandleEnergyDidReach0() {
        lastSummary->Died = true;
        if (auto controller = Alive(timeSyncController)) lastSummary->DeathTime = controller->_songTime;
        Play(Sounds::Sting(), 0.9f);
        Say("yours", true);
        ShowDeathText();
    }

    #pragma endregion

    #pragma region Lifecycle

    static void PrepareLevel(Zenject::DiContainer* container, GameplayCoreSceneSetupData* setupData) {
        if (!container || !setupData || !CustomModifiers::IsActive(DanceModifier::Id)) return;

        auto key = setupData->beatmapKey;
        if (!key.IsValid() || !DanceEvent::IsEventLevel(key)) return;
        eventLevel = true;

        flickerAllowed = true;
        if (auto settings = setupData->playerSpecificSettings) {
            auto filter = key.difficulty == BeatmapDifficulty::ExpertPlus
                ? settings->_environmentEffectsFilterExpertPlusPreset
                : settings->_environmentEffectsFilterDefaultPreset;
            flickerAllowed = filter == EnvironmentEffectsFilterPreset::AllEffects;
        }

        auto levelId = (std::string)key.levelId;
        levelHash = levelId.starts_with(CustomLevelPrefix) ? levelId.substr(CustomLevelPrefix.size()) : levelId;
        levelDifficulty = MapEnhancer::DiffName(key.difficulty.value__);
        levelMode = (std::string)BeatmapCharacteristicExtensions::SerializedName(key.characteristic);
        levelContainer = container;

        Lights::BeginLevel();
        Sounds::Preload();
        Apparitions::Preload();
        VignetteSprite();
        BeatLeaderLogger.info("Dance or Die: haunting {} {} {}, flicker {}", levelHash, levelDifficulty, levelMode, flickerAllowed);
    }

    void Prepare(Zenject::DiContainer* container, GameplayCoreSceneSetupData* setupData) {
        lastSummary = std::nullopt;
        eventLevel = false;
        levelContainer.clear();
        try {
            PrepareLevel(container, setupData);
        } catch (std::exception const& e) {
            BeatLeaderLogger.error("Dance or Die: can't prepare the level, playing without the effects: {}", e.what());
            eventLevel = false;
            levelContainer.clear();
        }
    }

    static void Start() {
        s = State();
        lastSummary = Summary();
        haunting = true;

        auto container = levelContainer.ptr();
        timeSyncController = Resolve<AudioTimeSyncController>(container);
        energyCounter = Resolve<GameEnergyCounter>(container);
        bpmController = Resolve<BpmController, IBpmController>(container);
        gamePause = Resolve<GamePause, IGamePause>(container);
        mainAudioEffects = Resolve<MainAudioEffects>(container);
        audioManager = Resolve<AudioManager>(container);
        hapticFeedbackManager = Resolve<HapticFeedbackManager>(container);
        if (!energyCounter) energyCounter = FindActive<GameEnergyCounter>();
        if (!mainAudioEffects) mainAudioEffects = FindActive<MainAudioEffects>();
        if (!hapticFeedbackManager) hapticFeedbackManager = FindActive<HapticFeedbackManager>();
        bloomFog = BeatLeader::FirstOrDefault(UnityEngine::Resources::FindObjectsOfTypeAll<BloomFogSO*>());
        BeatLeaderLogger.info("Dance or Die: the haunting starts (time {}, energy {}, bpm {}, pause {}, low pass {}, audio {}, haptics {}, fog {})",
            (bool)timeSyncController, (bool)energyCounter, (bool)bpmController, (bool)gamePause, (bool)mainAudioEffects,
            (bool)audioManager, (bool)hapticFeedbackManager, (bool)bloomFog);

        ReadBasePitch();
        Lights::CollectStaticLights();
        CreateAudio();
        Ghost::Load(levelHash, levelDifficulty, levelMode);
    }

    static bool IsPaused() {
        if (gamePause) return gamePause->_pause;
        auto controller = Alive(timeSyncController);
        return controller && controller->_state == IAudioTimeSource_State::Paused;
    }

    static void UpdateHaunting(Dance::DanceLive const& live) {
        if (!haunting) Start();

        auto counter = Alive(energyCounter);
        if (!lastSummary->Died && counter && counter->_isInitialized && counter->_energy_k__BackingField <= 0) {
            HandleEnergyDidReach0();
        }
        if (IsPaused()) return;

        UpdateDread(live);
        float dt = DeltaTime();
        s.shown += (s.dread - s.shown) * (1 - std::exp(-dt / EaseSeconds));

        auto controller = Alive(timeSyncController);
        float songTime = controller ? controller->_songTime : live.SongTime;
        if (s.shown > 0.5f && songTime > s.lastSongTime) {
            lastSummary->HauntedSeconds += songTime - s.lastSongTime;
        }
        lastSummary->PlayedSeconds = std::max(lastSummary->PlayedSeconds, songTime);
        s.lastSongTime = songTime;

        float lightsOut = Stage(LightsOut);
        UpdateLowPass(dt);
        UpdatePitch();
        UpdateLights(dt);
        UpdateVignette();
        UpdateFog(lightsOut);
        TrailDroop = 0.45f * lightsOut;
        NoteGray = 0.75f * Stage(0.1f);
        NoteDim = 0.5f * lightsOut;
        Apparitions::Update(lightsOut, Stage(TheDark), s.stops, NearDeath(), dt);
        Ghost::Update(lightsOut, songTime);
        warning = s.shown >= 0.85f;
        UpdateHeartbeat(songTime);
        UpdateWhispers(dt);
        UpdateVoice(dt);
        UpdateRelief();
    }

    void Update(Dance::DanceLive const& live) {
        if (!eventLevel) return;
        try {
            UpdateHaunting(live);
        } catch (std::exception const& e) {
            BeatLeaderLogger.error("Dance or Die: the effects failed, stopping them: {}", e.what());
            Stop();
        }
    }

    static void StopHaunting() {
        Apparitions::Dispose();
        Ghost::Dispose();
        Lights::Reset();
        RestorePitch();
        RestoreLowPass();
        RestoreFog();
        Destroy(vignette);
        vignetteImage = nullptr;
        DestroySource(audioSource);
        DestroySource(voiceSource);
        for (auto& source : whisperSources) DestroySource(source);
        Destroy(hapticPreset);

        timeSyncController = nullptr;
        energyCounter = nullptr;
        bpmController.clear();
        gamePause.clear();
        mainAudioEffects = nullptr;
        audioManager = nullptr;
        hapticFeedbackManager = nullptr;
        ForgetHead();
        BeatLeaderLogger.info("Dance or Die: died {}, haunted {:.1f} s of {:.1f} s, {} escapes, longest still {:.1f} s",
            lastSummary->Died, lastSummary->HauntedSeconds, lastSummary->PlayedSeconds, lastSummary->Escapes, lastSummary->LongestStillSeconds);
    }

    void Stop() {
        bool wasHaunting = haunting;
        eventLevel = false;
        levelContainer.clear();
        haunting = false;
        warning = false;
        TrailDroop = 0;
        NoteGray = 0;
        NoteDim = 0;
        try {
            if (wasHaunting) {
                StopHaunting();
            } else {
                Lights::Reset();
            }
        } catch (std::exception const& e) {
            BeatLeaderLogger.error("Dance or Die: can't put everything back: {}", e.what());
        }
    }

    #pragma endregion
}
