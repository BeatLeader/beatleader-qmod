#include "include/Assets/DanceEventAssets.hpp"
#include "assets.hpp"
#include "include/main.hpp"

#include "UnityEngine/Color32.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"

#include "bsml/shared/Helpers/utilities.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"

#include <zlib.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <span>
#include <string_view>
#include <vector>

using namespace UnityEngine;

namespace DanceEventAssets {
    #pragma region Embedded

    struct EmbeddedFile {
        std::string_view name;
        uint8_t* (*data)();
        size_t (*length)();
    };

    #define DANCE_EMBEDDED(file, symbol) EmbeddedFile { file, &symbol::getData, &symbol::getLength }

    static const EmbeddedFile embeddedFiles[] = {
        DANCE_EMBEDDED("bat-down.png", bat_down_png),
        DANCE_EMBEDDED("bat-up.png", bat_up_png),
        DANCE_EMBEDDED("candle-medium.png", candle_medium_png),
        DANCE_EMBEDDED("candle-short.png", candle_short_png),
        DANCE_EMBEDDED("candle-tall.png", candle_tall_png),
        DANCE_EMBEDDED("pumpkin.png", pumpkin_png),
        DANCE_EMBEDDED("pumpkin_cracked.obj.gz", pumpkin_cracked_obj_gz),
        DANCE_EMBEDDED("rip-tombstone.png", rip_tombstone_png),
        DANCE_EMBEDDED("skeleton_leaner.obj.gz", skeleton_leaner_obj_gz),
        DANCE_EMBEDDED("spider.png", spider_png),
        DANCE_EMBEDDED("web.png", web_png),
    };

    #undef DANCE_EMBEDDED

    static EmbeddedFile const* FindFile(std::string_view name) {
        for (auto const& file : embeddedFiles) {
            if (file.name == name) return &file;
        }
        BeatLeaderLogger.error("Dance event asset {} isn't embedded", name);
        return nullptr;
    }

    static std::map<std::pair<std::string, std::pair<float, float>>, safe_ptr<Sprite*>> sprites;

    static Sprite* CreateSprite(Texture2D* texture, Vector2 pivot) {
        if (!texture) return nullptr;
        texture->set_wrapMode(TextureWrapMode::Clamp);
        return Sprite::Create(texture, Rect(0, 0, texture->get_width(), texture->get_height()), pivot, 100.0f);
    }

    Sprite* GetSprite(std::string const& file, Vector2 pivot) {
        auto key = std::make_pair(file, std::make_pair(pivot.x, pivot.y));
        auto cached = sprites.find(key);
        if (cached != sprites.end() && cached->second) return cached->second.ptr();

        auto embedded = FindFile(file);
        if (!embedded) return nullptr;
        ArrayW<uint8_t> bytes(std::span<uint8_t const>(embedded->data(), embedded->length()));
        auto sprite = CreateSprite(BSML::Utilities::LoadTextureRaw(bytes), pivot);
        if (!sprite) {
            BeatLeaderLogger.error("Couldn't load {}", file);
            return nullptr;
        }
        sprites[key] = sprite;
        return sprite;
    }

    std::optional<std::string> Gunzip(std::string const& file) {
        constexpr int GzipWindowBits = 16 + MAX_WBITS;

        auto embedded = FindFile(file);
        if (!embedded) return std::nullopt;

        z_stream stream {};
        stream.next_in = embedded->data();
        stream.avail_in = (uInt)embedded->length();
        if (inflateInit2(&stream, GzipWindowBits) != Z_OK) return std::nullopt;

        std::string result;
        std::vector<char> buffer(1 << 16);
        int status;
        do {
            stream.next_out = reinterpret_cast<Bytef*>(buffer.data());
            stream.avail_out = (uInt)buffer.size();
            status = inflate(&stream, Z_NO_FLUSH);
            if (status != Z_OK && status != Z_STREAM_END) {
                inflateEnd(&stream);
                BeatLeaderLogger.error("Couldn't unpack {}: {}", file, status);
                return std::nullopt;
            }
            result.append(buffer.data(), buffer.size() - stream.avail_out);
        } while (status != Z_STREAM_END);
        inflateEnd(&stream);
        return result;
    }

    #pragma endregion

    #pragma region Noise

    static int Permutation(int i) {
        static const uint8_t table[256] = {
            151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
            190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,
            68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
            102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,
            3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
            223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,
            112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
            184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
        };
        return table[i & 255];
    }

    static float Fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static float Gradient(int hash, float x, float y) {
        switch (hash & 7) {
            case 0: return x + y;
            case 1: return -x + y;
            case 2: return x - y;
            case 3: return -x - y;
            case 4: return x;
            case 5: return -x;
            case 6: return y;
            default: return -y;
        }
    }

    float PerlinNoise(float x, float y) {
        int xi = (int)std::floor(x);
        int yi = (int)std::floor(y);
        float xf = x - xi;
        float yf = y - yi;
        float u = Fade(xf);
        float v = Fade(yf);
        int aa = Permutation(Permutation(xi) + yi);
        int ab = Permutation(Permutation(xi) + yi + 1);
        int ba = Permutation(Permutation(xi + 1) + yi);
        int bb = Permutation(Permutation(xi + 1) + yi + 1);
        float x1 = std::lerp(Gradient(aa, xf, yf), Gradient(ba, xf - 1, yf), u);
        float x2 = std::lerp(Gradient(ab, xf, yf - 1), Gradient(bb, xf - 1, yf - 1), u);
        return std::clamp(0.5f + 0.5f * std::lerp(x1, x2, v), 0.0f, 1.0f);
    }

    #pragma endregion

    #pragma region Generated

    static float Clamp01(float value) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    static float SmoothStep01(float t) {
        t = Clamp01(t);
        return t * t * (3 - 2 * t);
    }

    static Color32 Pixel(float r, float g, float b, float a) {
        return Color32(0, (uint8_t)(Clamp01(r) * 255), (uint8_t)(Clamp01(g) * 255), (uint8_t)(Clamp01(b) * 255), (uint8_t)(Clamp01(a) * 255));
    }

    static Sprite* Create(int width, int height, std::vector<Color32> const& pixels, Vector2 pivot) {
        auto texture = Texture2D::New_ctor(width, height, TextureFormat::RGBA32, false);
        texture->SetPixels32(ArrayW<Color32>(pixels));
        texture->Apply();
        return CreateSprite(texture, pivot);
    }

    template<Sprite* (*create)()>
    static Sprite* Cached() {
        static safe_ptr<Sprite*> sprite;
        if (!sprite) sprite = create();
        return sprite.ptr();
    }

    static Sprite* CreateFlame() {
        const int width = 64, height = 128;
        std::vector<Color32> pixels(width * height, Pixel(0, 0, 0, 0));
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float u = (x + 0.5f) / width * 2 - 1;
                float v = (y + 0.5f) / height;
                float halfWidth = v < 0.25f
                    ? 0.8f * std::sqrt(std::max(0.0f, 1 - std::pow((0.25f - v) / 0.25f, 2.0f)))
                    : 0.8f * std::pow(1 - (v - 0.25f) / 0.75f, 1.4f);
                if (halfWidth <= 0.001f) continue;
                float d = std::abs(u) / halfWidth;
                if (d >= 1) continue;
                float a = 1 - SmoothStep01((d - 0.55f) / 0.45f);
                float heat = Clamp01(1 - d) * Clamp01(1.2f - v);
                pixels[y * width + x] = Pixel(1, std::lerp(0.5f, 0.96f, heat), std::lerp(0.1f, 0.78f, heat), a);
            }
        }
        return Create(width, height, pixels, {0.5f, 0.12f});
    }

    static Sprite* CreateGlow() {
        const int size = 128;
        std::vector<Color32> pixels(size * size);
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                float dx = (x + 0.5f) / size * 2 - 1;
                float dy = (y + 0.5f) / size * 2 - 1;
                float a = Clamp01(1 - std::sqrt(dx * dx + dy * dy));
                pixels[y * size + x] = Pixel(1, 1, 1, a * a * a);
            }
        }
        return Create(size, size, pixels, {0.5f, 0.5f});
    }

    static Sprite* CreateMist() {
        const int size = 256;
        std::vector<Color32> pixels(size * size);
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                float u = (float)x / size;
                float v = (float)y / size;
                float noise = 0.6f * PerlinNoise(u * 4, v * 4) + 0.4f * PerlinNoise(u * 9 + 13, v * 9 + 7);
                float dx = u * 2 - 1;
                float dy = v * 2 - 1;
                float edge = Clamp01(1 - std::sqrt(dx * dx + dy * dy));
                float a = Clamp01((noise - 0.35f) * 2.2f) * SmoothStep01(edge * 1.6f);
                pixels[y * size + x] = Pixel(1, 1, 1, a);
            }
        }
        return Create(size, size, pixels, {0.5f, 0.5f});
    }

    static Sprite* CreateFog() {
        const int width = 256, height = 64;
        std::vector<Color32> pixels(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float v = (float)y / height;
                float wisps = 0.65f + 0.35f * PerlinNoise(x * 0.035f, y * 0.09f);
                float a = Clamp01(1 - v) * Clamp01(1 - v) * wisps;
                pixels[y * width + x] = Pixel(1, 1, 1, a);
            }
        }
        return Create(width, height, pixels, {0.5f, 0});
    }

    static Sprite* CreateThread() {
        return Create(1, 1, {Pixel(1, 1, 1, 1)}, {0.5f, 1});
    }

    Sprite* Flame() { return Cached<CreateFlame>(); }
    Sprite* Glow() { return Cached<CreateGlow>(); }
    Sprite* Mist() { return Cached<CreateMist>(); }
    Sprite* Fog() { return Cached<CreateFog>(); }
    Sprite* Thread() { return Cached<CreateThread>(); }

    #pragma endregion
}
