#include "include/Core/Dance/DanceDreadParts.hpp"

#include "UnityEngine/Color32.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"

#include "bsml/shared/Helpers/utilities.hpp"

#include "assets.hpp"
#include "main.hpp"

using namespace UnityEngine;

namespace DanceDread::Apparitions {
    static constexpr Color EyeColor = {1, 0.28f, 0.06f, 1};

    #pragma region Sprites

    static constexpr int AuraPadding = 40;

    static SafePtrUnity<Sprite> silhouetteSprite;
    static SafePtrUnity<Sprite> auraSprite;
    static bool silhouetteFailed = false;

    static Sprite* EyesSprite() {
        static SafePtrUnity<Sprite> sprite;
        if (!sprite) {
            sprite = CreateAlphaSprite(192, 72, {0.5f, 0.5f}, [](float u, float v) {
                v = v * 2 - 1;
                float alpha = 0;
                for (float center : {0.27f, 0.73f}) {
                    float dx = (u - center) / 0.19f;
                    float halfHeight = 0.8f * (1 - dx * dx);
                    if (halfHeight <= 0) continue;
                    float eye = Clamp01((1 - std::abs(v) / halfHeight) * 3);
                    float pupil = Clamp01(1 - std::abs(dx) / 0.09f) * 0.85f;
                    float glow = Clamp01(1 - std::sqrt(dx * dx / 1.96f + v * v)) * 0.35f;
                    alpha = std::max(alpha, std::max(eye * (1 - pupil), glow));
                }
                return alpha;
            });
        }
        return Alive(sprite);
    }

    static void BoxBlur(std::vector<float>& data, int width, int height, int radius, bool horizontal) {
        int count = horizontal ? height : width;
        int length = horizontal ? width : height;
        std::vector<float> line(length);
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < length; j++) line[j] = data[horizontal ? i * width + j : j * width + i];
            float sum = 0;
            for (int j = -radius; j <= radius; j++) sum += j >= 0 && j < length ? line[j] : 0;
            for (int j = 0; j < length; j++) {
                data[horizontal ? i * width + j : j * width + i] = sum / (2 * radius + 1);
                int add = j + radius + 1;
                int remove = j - radius;
                if (add < length) sum += line[add];
                if (remove >= 0) sum -= line[remove];
            }
        }
    }

    static Sprite* CreateAura(Texture2D* source) {
        auto pixels = source->GetPixels32();
        int w = source->width, h = source->height, p = AuraPadding;
        int width = w + 2 * p, height = h + 2 * p;
        std::vector<float> body(width * height);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) body[(y + p) * width + x + p] = pixels[y * w + x].a / 255.0f;
        }
        auto glow = body;
        for (int pass = 0; pass < 3; pass++) {
            BoxBlur(glow, width, height, 12, true);
            BoxBlur(glow, width, height, 12, false);
        }
        for (size_t i = 0; i < glow.size(); i++) glow[i] = Clamp01(glow[i] * 2.2f) * (1 - body[i]);
        return CreateAlphaSprite(width, height, {0.5f, 0}, glow);
    }

    static Sprite* SilhouetteSprite() {
        if (silhouetteSprite || silhouetteFailed) return Alive(silhouetteSprite);
        ArrayW<uint8_t> bytes(silhouette_png::getLength());
        std::copy(silhouette_png::getData(), silhouette_png::getData() + silhouette_png::getLength(), bytes.begin());
        auto texture = BSML::Utilities::LoadTextureRaw(bytes);
        if (!texture) {
            silhouetteFailed = true;
            BeatLeaderLogger.warn("Dance or Die: can't load silhouette.png");
            return nullptr;
        }
        silhouetteSprite = CreateSprite(texture, {0.5f, 0});
        auraSprite = CreateAura(texture);
        return Alive(silhouetteSprite);
    }

    void Preload() {
        EyesSprite();
        SilhouetteSprite();
    }

    #pragma endregion

    #pragma region Eyes

    struct EyePlace {
        float angle, distance, height, threshold;
    };

    static constexpr EyePlace EyePlaces[] = {
        {-62, 26, 1.4f, 0},
        {48, 31, 2.6f, 0.15f},
        {-34, 34, 3.4f, 0.3f},
        {74, 22, 0.9f, 0.45f},
        {-80, 20, 2.1f, 0.6f}
    };

    struct EyePair {
        SafePtrUnity<GameObject> root;
        SafePtrUnity<UI::Image> image;
        float threshold = 0;
        float blinkTimer = 0;
        float nextBlink = 0;
        float appliedAlpha = -1;
        bool closed = false;
    };

    static std::vector<EyePair> eyes;

    static void UpdateEyes(float stage, Vector3 head, float dt) {
        if (eyes.empty()) {
            if (stage <= 0) return;
            for (auto const& place : EyePlaces) {
                auto [root, image] = CreateImage("DanceDreadEyes", EyesSprite(), {1.2f, 0.45f}, nullptr);
                root->transform->position = Add(Scale(AroundY(place.angle), place.distance), {0, place.height, 0});
                eyes.push_back({.root = root, .image = image, .threshold = place.threshold, .nextBlink = RandomRange(2.0f, 6.0f)});
                image->color = WithAlpha(EyeColor, 0);
            }
        }
        for (auto& pair : eyes) {
            auto root = Alive(pair.root);
            auto image = Alive(pair.image);
            if (!root || !image) continue;
            float alpha = Clamp01((stage - pair.threshold) / 0.25f);
            if (std::abs(alpha - pair.appliedAlpha) > 0.004f) {
                pair.appliedAlpha = alpha;
                image->color = WithAlpha(EyeColor, alpha);
            }
            if (alpha <= 0) continue;
            Face(root->transform, head);
            pair.blinkTimer += dt;
            bool closed = pair.blinkTimer >= pair.nextBlink;
            if (pair.blinkTimer >= pair.nextBlink + 0.14f) {
                pair.blinkTimer = 0;
                pair.nextBlink = RandomRange(2.5f, 7.0f);
            }
            if (closed != pair.closed) {
                pair.closed = closed;
                root->transform->localScale = {ImageScale, (closed ? 0.1f : 1) * ImageScale, ImageScale};
            }
        }
    }

    #pragma endregion

    #pragma region Silhouette

    static constexpr float SilhouetteHeight = 2.3f;
    static constexpr float SilhouetteEyesY = 0.943f;
    static constexpr float StartSilhouetteDistance = 22;
    static constexpr float MinSilhouetteDistance = 4;
    static constexpr float DeathDistance = 1.5f;

    static constexpr Color AuraColor = {0.75f, 0.12f, 0.3f, 1};

    static SafePtrUnity<GameObject> silhouette;
    static SafePtrUnity<UI::Image> silhouetteImage;
    static SafePtrUnity<UI::Image> silhouetteEyes;
    static SafePtrUnity<UI::Image> aura;

    struct SilhouetteState {
        float distance = StartSilhouetteDistance;
        int placedStops = 0;
        float jumpTimer = 0;
        float nextJump = 0;
        float hidden = 0;
        Vector3 nextSpot = {0, 0, 0};
        float flare = 0;
        bool closingIn = false;
        float dropout = 0;
        float nextDropout = 1;
    };

    static SilhouetteState state;

    static void CreateSilhouette() {
        auto sprite = SilhouetteSprite();
        auto rect = sprite->rect;
        float width = SilhouetteHeight * rect.m_Width / rect.m_Height;
        auto [root, image] = CreateImage("DanceDreadSilhouette", sprite, {width, SilhouetteHeight}, NoGlowMaterial());
        root->GetComponent<RectTransform*>()->pivot = {0.5f, 0};
        root->transform->position = {0, 0, state.distance};
        image->color = {1, 1, 1, 0};

        if (auto auraSource = Alive(auraSprite)) {
            auto glow = AddImage("Aura", auraSource, root->transform, nullptr);
            glow->transform->SetAsFirstSibling();
            float padding = AuraPadding * (SilhouetteHeight / ImageScale) / rect.m_Height;
            auto glowRect = glow->rectTransform;
            glowRect->anchorMin = {0, 0};
            glowRect->anchorMax = {1, 1};
            glowRect->offsetMin = {-padding, -padding};
            glowRect->offsetMax = {padding, padding};
            glow->color = WithAlpha(AuraColor, 0);
            aura = glow;
        }

        auto eyeImage = AddImage("Eyes", EyesSprite(), image->transform, nullptr);
        auto eyeRect = eyeImage->rectTransform;
        eyeRect->anchorMin = {0.5f, SilhouetteEyesY};
        eyeRect->anchorMax = {0.5f, SilhouetteEyesY};
        eyeRect->sizeDelta = {0.32f / ImageScale, 0.12f / ImageScale};
        eyeImage->color = WithAlpha(EyeColor, 0);

        silhouette = root;
        silhouetteImage = image;
        silhouetteEyes = eyeImage;
    }

    static Vector3 PickSpot(float stage, float nearDeath, Vector3 head) {
        Vector3 feet = {head.x, 0, head.z};
        if (nearDeath > 0) {
            return Add(feet, {RandomRange(-0.35f, 0.35f), 0, DeathDistance});
        }
        float distance = std::max(3.0f, state.distance * Lerp(1, 0.55f, stage) * RandomRange(0.85f, 1.1f));
        float side = RandomValue() < 0.5f ? -1 : 1;
        float angle = RandomValue() < 0.3f ? side * RandomRange(35.0f, 60.0f) : RandomRange(-15.0f, 15.0f);
        return Add(feet, Scale(AroundY(angle), distance));
    }

    static void UpdateSilhouette(float stage, int stops, float nearDeath, Vector3 head, float dt) {
        if (!silhouette) {
            if (stage <= 0 || !SilhouetteSprite()) return;
            CreateSilhouette();
        }

        bool stopped = stops != state.placedStops;
        if (stopped) {
            state.placedStops = stops;
            state.distance = std::max(MinSilhouetteDistance, StartSilhouetteDistance * std::pow(0.8f, (float)std::max(0, stops - 1)));
        }
        bool closingIn = nearDeath > 0;
        state.jumpTimer += dt;
        if (stage > 0.05f && state.hidden <= 0 && (stopped || state.jumpTimer >= state.nextJump || closingIn != state.closingIn)) {
            state.jumpTimer = 0;
            state.nextJump = closingIn ? RandomRange(0.5f, 0.9f) : Lerp(7, 2.2f, stage) * RandomRange(0.7f, 1.3f);
            state.nextSpot = PickSpot(stage, nearDeath, head);
            state.hidden = RandomRange(0.12f, 0.35f);
        }
        state.closingIn = closingIn;
        auto transform = silhouette->transform;
        if (state.hidden > 0) {
            state.hidden -= dt;
            if (state.hidden <= 0) {
                transform->position = state.nextSpot;
                state.flare = 1;
            }
        }

        state.nextDropout -= dt;
        if (state.dropout > 0) {
            state.dropout -= dt;
        } else if (state.nextDropout <= 0) {
            state.dropout = RandomRange(0.04f, 0.12f);
            state.nextDropout = RandomRange(0.3f, 2.5f) * Lerp(1.5f, 0.4f, std::max(stage, nearDeath));
        }
        state.flare = std::max(0.0f, state.flare - dt * 2.5f);
        float visibility = state.hidden > 0 ? 0 : state.dropout > 0 ? 0.15f : 1;
        float alpha = Clamp01(std::max(stage * 1.2f, nearDeath)) * visibility;
        silhouetteImage->color = {1, 1, 1, alpha};
        silhouetteEyes->color = WithAlpha(EyeColor, Clamp01(alpha * alpha + state.flare * visibility));
        if (auto glow = Alive(aura)) {
            float time = GameTime() * 6;
            float shimmer = 0.75f + 0.25f * (0.5f + 0.25f * std::sin(time * 1.3f) + 0.25f * std::sin(time * 2.9f + 1.7f));
            glow->color = WithAlpha(AuraColor, Clamp01((0.9f * shimmer + 0.6f * state.flare) * alpha));
        }
        if (alpha > 0) Face(transform, head);
    }

    #pragma endregion

    void Update(float silhouetteStage, float eyesStage, int stops, float nearDeath, float dt) {
        if (silhouetteStage <= 0 && eyesStage <= 0 && eyes.empty() && !silhouette) return;
        if (!Head()) return;
        auto head = HeadPosition();
        UpdateEyes(eyesStage, head, dt);
        UpdateSilhouette(silhouetteStage, stops, nearDeath, head, dt);
    }

    void Dispose() {
        for (auto& pair : eyes) Destroy(pair.root);
        eyes.clear();
        Destroy(silhouette);
        silhouetteImage = nullptr;
        silhouetteEyes = nullptr;
        aura = nullptr;
        state = SilhouetteState();
    }
}
