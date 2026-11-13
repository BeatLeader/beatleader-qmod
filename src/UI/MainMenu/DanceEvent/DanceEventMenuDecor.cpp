#include "UI/MainMenu/DanceEvent/DanceEventMenuDecor.hpp"
#include "UI/MainMenu/DanceEvent/DanceEventModels.hpp"
#include "UI/MainMenu/DanceEvent/DanceEventPanel.hpp"
#include "Assets/DanceEventAssets.hpp"
#include "Core/Dance/DanceEvent.hpp"
#include "main.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/ImageView.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/CanvasRenderer.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Space.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Image.hpp"

#include "sombrero/shared/FastQuaternion.hpp"
#include "sombrero/shared/FastVector2.hpp"
#include "sombrero/shared/FastVector3.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"
#include "bsml/shared/Helpers/getters.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <numbers>
#include <random>

DEFINE_TYPE(BeatLeader, DanceEventMenuDecor);

using namespace UnityEngine;
using Sombrero::FastQuaternion;
using Sombrero::FastVector2;
using Sombrero::FastVector3;

namespace {
    #pragma region Utils

    constexpr float Deg2Rad = std::numbers::pi_v<float> / 180;
    constexpr float WorldImageScale = 0.01f;
    const int ImageViewCurveChannel = AdditionalCanvasShaderChannels::TexCoord2.value__;
    constexpr int BottomRightCorner = 3;
    constexpr int TopRightCorner = 2;
    const FastVector3 DefaultHead = FastVector3(0, 1.7f, 0);

    using Random = std::mt19937;

    float Chance(Random& random) {
        return std::uniform_real_distribution<float>(0, 1)(random);
    }

    float Range(Random& random, float from, float to) {
        return from + Chance(random) * (to - from);
    }

    int Next(Random& random, int from, int to) {
        return std::uniform_int_distribution<int>(from, to - 1)(random);
    }

    float Clamp01(float value) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    Color WithAlpha(Color color, float alpha) {
        return Color(color.r, color.g, color.b, alpha);
    }

    Color Scaled(Color color, float factor) {
        return Color(color.r * factor, color.g * factor, color.b * factor, color.a * factor);
    }

    struct Spot {
        FastVector2 spot;
        float radius;
    };

    bool Overlaps(Spot const& other, FastVector2 candidate, float radius) {
        return (other.spot - candidate).Magnitude() < other.radius + radius;
    }

    bool TryPlace(Random& random, std::vector<Spot>& taken, float fromDegrees, float toDegrees, float near, float far, float radius, FastVector2& spot) {
        for (int attempt = 0; attempt < 40; attempt++) {
            float angle = Deg2Rad * Range(random, fromDegrees, toDegrees);
            float distance = Range(random, near, far);
            FastVector2 candidate(std::sin(angle) * distance, std::cos(angle) * distance);
            if (std::ranges::any_of(taken, [&](Spot const& other) { return Overlaps(other, candidate, radius); })) continue;
            taken.push_back({candidate, radius});
            spot = candidate;
            return true;
        }
        return false;
    }

    FastVector3 OnFloor(FastVector2 spot, float height = 0) {
        return FastVector3(spot.x, height, spot.y);
    }

    float CandleFlicker(float time) {
        float breath = DanceEventAssets::PerlinNoise(time * 0.7f, 0.3f);
        float flutter = DanceEventAssets::PerlinNoise(time * 9, 5.1f);
        return Clamp01(0.45f + 0.35f * breath + 0.25f * flutter);
    }

    void FaceHead(Transform* transform, FastVector3 head, float centerHeight, float maxLean) {
        FastVector3 away = FastVector3(transform->get_position()) + FastVector3(0, centerHeight, 0) - head;
        FastVector3 flat(away.x, 0, away.z);
        if (flat.sqrMagnitude() < 0.0001f) return;
        float lean = std::min(maxLean, Vector3::Angle(flat, away));
        transform->set_rotation(FastQuaternion(Quaternion::LookRotation(flat)) * FastQuaternion(Quaternion::Euler(lean, 0, 0)));
    }

    UI::Image* AddImage(Transform* parent, std::string const& name, Sprite* sprite, Material* material) {
        auto go = GameObject::New_ctor(name);
        // A plain Image draws nothing with the game's UI materials
        UI::Image* image = material ? (UI::Image*)go->AddComponent<HMUI::ImageView*>() : go->AddComponent<UI::Image*>();
        image->get_transform()->SetParent(parent, false);
        image->set_sprite(sprite);
        image->set_raycastTarget(false);
        if (material) image->set_material(material);
        return image;
    }

    void Stretch(RectTransform* rect) {
        rect->set_anchorMin({0, 0});
        rect->set_anchorMax({1, 1});
        rect->set_sizeDelta({0, 0});
    }

    void AddShaderChannels(Canvas* canvas, int channels) {
        canvas->set_additionalShaderChannels(AdditionalCanvasShaderChannels(canvas->get_additionalShaderChannels().value__ | channels));
    }

    UI::Image* CreateWorldImage(std::string const& name, Sprite* sprite, FastVector2 size, Material* material, GameObject*& root) {
        root = GameObject::New_ctor(name);
        auto canvas = root->AddComponent<Canvas*>();
        canvas->set_renderMode(RenderMode::WorldSpace);
        AddShaderChannels(canvas, ImageViewCurveChannel);
        auto rect = root->GetComponent<RectTransform*>();
        rect->set_sizeDelta(size / WorldImageScale);
        rect->set_localScale(FastVector3(WorldImageScale, WorldImageScale, WorldImageScale));
        auto image = AddImage(rect, "Image", sprite, material);
        Stretch(image->get_rectTransform());
        return image;
    }

    #pragma endregion

    #pragma region State

    struct Candle {
        SafePtrUnity<Transform> root;
        SafePtrUnity<UI::Image> flame;
        float phase;
        float center;
    };

    struct ImagePhase {
        SafePtrUnity<UI::Image> image;
        float phase;
    };

    struct Glow {
        SafePtrUnity<Material> material;
        Color color;
    };

    struct Pumpkin {
        SafePtrUnity<Transform> root;
        float phase;
        float yaw;
        std::vector<Glow> glows;
    };

    struct Mist {
        SafePtrUnity<Transform> root;
        float degreesPerSecond;
    };

    struct Web {
        SafePtrUnity<HMUI::Screen> screen;
        SafePtrUnity<Canvas> canvas;
        SafePtrUnity<RectTransform> rect;
        bool rightCorner;
    };

    struct Bat {
        SafePtrUnity<Transform> root;
        SafePtrUnity<UI::Image> image;
        FastVector3 center;
        float radius;
        float speed;
        float angle;
        float flap;
        float phase;
        bool wingsUp = true;
    };

    struct DecorState {
        SafePtrUnity<GameObject> root;
        std::vector<SafePtrUnity<GameObject>> screenDecor;
        SafePtrUnity<Camera> camera;

        std::vector<Pumpkin> pumpkins;
        std::vector<Candle> candles;
        std::vector<ImagePhase> candleLights;
        std::vector<Mist> mist;

        std::vector<Web> webs;
        float nextWebLayout = 0;
        bool websShown = false;
        SafePtrUnity<RectTransform> thread;
        float threadPhase = 0;

        std::vector<Bat> bats;
        SafePtrUnity<Sprite> batUp;
        SafePtrUnity<Sprite> batDown;

        SafePtrUnity<GameObject> skeleton;
        SafePtrUnity<RectTransform> skeletonPanel;
        SafePtrUnity<HMUI::CurvedCanvasSettings> skeletonCurve;

        SafePtr<Array<Vector3>> corners;
    };

    std::unique_ptr<DecorState> state;
    SafePtrUnity<BeatLeader::DanceEventMenuDecor> decorInstance;
    SafePtrUnity<GlobalNamespace::MainMenuViewController> mainMenuView;

    bool MainMenuShown() {
        return mainMenuView && mainMenuView->get_isActivated();
    }

    ArrayW<Vector3> Corners() {
        if (!state->corners) state->corners = static_cast<Array<Vector3>*>(ArrayW<Vector3>(4));
        return ArrayW<Vector3>(state->corners.ptr());
    }

    GameObject* CreateModel(std::string const& name, DanceEventModels::Model const& model, std::vector<Material*> const& materials) {
        auto go = GameObject::New_ctor(name);
        go->get_transform()->SetParent(state->root->get_transform(), false);
        go->AddComponent<MeshFilter*>()->set_sharedMesh(model.mesh.ptr());
        go->AddComponent<MeshRenderer*>()->set_sharedMaterials(ArrayW<Material*>(materials));
        return go;
    }

    #pragma endregion

    #pragma region Pumpkins

    constexpr float PumpkinModelWidth = 0.5f;

    int ColorId() {
        static int id = Shader::PropertyToID("_Color");
        return id;
    }

    void SpawnPumpkins(Random& random, std::vector<Spot>& taken) {
        auto model = DanceEventModels::Get("pumpkin_cracked.obj.gz");
        if (!model) return;

        auto glowColor = DanceEventModels::Hex("FFB347");
        auto carvedColor = DanceEventModels::Hex("C84A00");
        auto shell = DanceEventModels::Solid(DanceEventModels::Hex("CF6817"), 0.3f);
        auto stem = DanceEventModels::Solid(DanceEventModels::Hex("4F4A22"), 0.15f);
        if (!shell || !stem) return;

        auto addPumpkin = [&](FastVector2 spot, float size) {
            Pumpkin pumpkin { .phase = Range(random, 0, 100), .yaw = Range(random, -30, 30) };
            std::vector<Material*> materials;
            for (auto const& name : model->materials) {
                if (name == "glow" || name == "carved") {
                    auto color = name == "glow" ? glowColor : carvedColor;
                    auto material = DanceEventModels::Glowing(color);
                    if (!material) return;
                    pumpkin.glows.push_back({material, color});
                    materials.push_back(material);
                } else {
                    materials.push_back(name == "stem" ? stem : shell);
                }
            }
            auto go = CreateModel("Pumpkin", *model, materials);
            go->get_transform()->set_position(OnFloor(spot));
            float scale = size / PumpkinModelWidth;
            go->get_transform()->set_localScale(FastVector3(scale, scale, scale));
            pumpkin.root = go->get_transform().ptr();
            state->pumpkins.push_back(std::move(pumpkin));
        };

        int count = Next(random, 5, 8);
        for (int attempt = 0; attempt < 20 && (int)state->pumpkins.size() < count; attempt++) {
            bool front = Chance(random) < 0.7f;
            float flank = Next(random, 0, 2) == 0 ? -1.0f : 1.0f;
            FastVector2 spot;
            bool placed = front
                ? TryPlace(random, taken, -75, 75, 6, 10, 0.6f, spot)
                : TryPlace(random, taken, flank * 80, flank * 180, 6, 8.5f, 0.6f, spot);
            if (!placed) continue;
            float size = Range(random, 0.4f, 0.7f) * (0.8f + 0.1f * spot.Magnitude());
            addPumpkin(spot, size);
            if (size < 0.55f || Chance(random) > 0.45f) continue;

            float small = Range(random, 0.26f, 0.36f);
            float side = Chance(random) * std::numbers::pi_v<float> * 2;
            FastVector2 next = spot + FastVector2(std::cos(side), std::sin(side)) * (size * 0.5f + small * 0.55f);
            bool blocked = std::ranges::any_of(taken, [&](Spot const& other) {
                return (other.spot - spot).Magnitude() > 0.0001f && Overlaps(other, next, small * 0.5f);
            });
            if (blocked) continue;
            taken.push_back({next, small * 0.5f});
            addPumpkin(next, small);
        }
    }

    void UpdatePumpkins(FastVector3 head, float time) {
        for (auto& pumpkin : state->pumpkins) {
            if (!pumpkin.root) continue;
            auto root = pumpkin.root.ptr();
            FastVector3 away = FastVector3(root->get_position()) - head;
            away.y = 0;
            if (away.sqrMagnitude() > 0.01f) {
                root->set_rotation(FastQuaternion(Quaternion::LookRotation(-away)) * FastQuaternion(Quaternion::Euler(0, pumpkin.yaw, 0)));
            }
            float flicker = CandleFlicker(time + pumpkin.phase);
            for (auto& glow : pumpkin.glows) {
                if (glow.material) glow.material->SetColor(ColorId(), Scaled(glow.color, 0.45f + 0.75f * flicker));
            }
        }
    }

    #pragma endregion

    #pragma region Candles

    struct CandleVariant {
        const char* file;
        float bottom;
        FastVector2 wick;
    };

    const FastVector2 CandleSpriteSize = FastVector2(0.5f, 1.25f);
    const std::array<CandleVariant, 3> CandleVariants = {{
        {"candle-tall.png", 0.04f, FastVector2(0.51f, 0.875f)},
        {"candle-medium.png", 0.035f, FastVector2(0.5f, 0.655f)},
        {"candle-short.png", 0.0275f, FastVector2(0.5f, 0.45f)}
    }};
    const Color CandleLightColor = Color(1, 0.55f, 0.2f, 1);

    void AddCandle(Random& random, FastVector2 spot, CandleVariant const& variant, float scale) {
        auto sprite = DanceEventAssets::GetSprite(variant.file, {0.5f, variant.bottom});
        if (!sprite) return;
        GameObject* root;
        auto body = CreateWorldImage("Candle", sprite, CandleSpriteSize * scale, BSML::Helpers::GetUINoGlowMat(), root);
        root->GetComponent<RectTransform*>()->set_pivot({0.5f, variant.bottom});
        root->get_transform()->SetParent(state->root->get_transform(), false);
        root->get_transform()->set_position(OnFloor(spot));

        auto flame = AddImage(body->get_transform(), "Flame", DanceEventAssets::Flame(), nullptr);
        auto rect = flame->get_rectTransform();
        rect->set_anchorMin(variant.wick);
        rect->set_anchorMax(variant.wick);
        rect->set_pivot({0.5f, 0.12f});
        rect->set_sizeDelta(FastVector2(0.12f, 0.26f) * (scale / WorldImageScale));
        state->candles.push_back({root->get_transform().ptr(), flame, Range(random, 0, 100), scale * 0.4f});
    }

    void SpawnCandles(Random& random, std::vector<Spot>& taken) {
        int clusters = Next(random, 3, 5);
        for (int c = 0; c < clusters; c++) {
            FastVector2 center;
            if (!TryPlace(random, taken, -70, 70, 3, 3.9f, 0.4f, center)) break;
            std::vector<FastVector2> placed;
            int count = Next(random, 2, 5);
            for (int i = 0; i < count; i++) {
                for (int attempt = 0; attempt < 20; attempt++) {
                    FastVector2 offset(Range(random, -0.2f, 0.2f), Range(random, -0.14f, 0.14f));
                    if (std::ranges::any_of(placed, [&](FastVector2 other) { return (other - offset).Magnitude() < 0.12f; })) continue;
                    placed.push_back(offset);
                    auto const& variant = CandleVariants[Next(random, 0, (int)CandleVariants.size())];
                    AddCandle(random, center + offset, variant, Range(random, 0.3f, 0.42f));
                    break;
                }
            }
            GameObject* lightRoot;
            float lightSize = Range(random, 1.1f, 1.5f);
            auto light = CreateWorldImage("CandleLight", DanceEventAssets::Glow(), FastVector2(lightSize, lightSize), nullptr, lightRoot);
            lightRoot->get_transform()->SetParent(state->root->get_transform(), false);
            lightRoot->get_transform()->set_position(OnFloor(center, 0.01f));
            lightRoot->get_transform()->set_rotation(Quaternion::Euler(90, 0, 0));
            state->candleLights.push_back({light, Range(random, 0, 100)});
        }
    }

    void UpdateCandles(FastVector3 head, float time) {
        for (auto& candle : state->candles) {
            if (!candle.root || !candle.flame) continue;
            FaceHead(candle.root.ptr(), head, candle.center, 20);
            float t = time + candle.phase;
            float stretch = DanceEventAssets::PerlinNoise(t * 7, 1.3f);
            float sway = DanceEventAssets::PerlinNoise(t * 1.7f, 8.2f) - 0.5f;
            auto flame = candle.flame->get_rectTransform();
            flame->set_localScale(FastVector3(1.05f - 0.12f * stretch, 0.8f + 0.4f * stretch, 1));
            flame->set_localRotation(Quaternion::Euler(0, 0, sway * 10));
            candle.flame->set_color(Color(1, 1, 1, 0.8f + 0.2f * DanceEventAssets::PerlinNoise(t * 11, 4.4f)));
        }
        for (auto& light : state->candleLights) {
            if (!light.image) continue;
            light.image->set_color(WithAlpha(CandleLightColor, 0.22f * (0.6f + 0.4f * CandleFlicker(time + light.phase))));
        }
    }

    #pragma endregion

    #pragma region Mist

    struct MistLayer {
        float height;
        float size;
        float degreesPerSecond;
    };

    const std::array<MistLayer, 3> MistLayers = {{
        {0.06f, 9, 2.5f},
        {0.18f, 7, -3.5f},
        {0.32f, 11, 1.5f}
    }};
    const Color MistColor = Color(0.62f, 0.55f, 0.78f, 0.07f);

    void SpawnMist(Random& random) {
        for (auto const& layer : MistLayers) {
            GameObject* root;
            auto image = CreateWorldImage("Mist", DanceEventAssets::Mist(), FastVector2(layer.size, layer.size), nullptr, root);
            root->get_transform()->SetParent(state->root->get_transform(), false);
            root->get_transform()->set_position(FastVector3(0, layer.height, 0));
            root->get_transform()->set_rotation(Quaternion::Euler(90, Range(random, 0, 360), 0));
            image->set_color(MistColor);
            state->mist.push_back({root->get_transform().ptr(), layer.degreesPerSecond});
        }
    }

    void UpdateMist(float deltaTime) {
        for (auto& layer : state->mist) {
            if (layer.root) layer.root->Rotate(0, 0, layer.degreesPerSecond * deltaTime, Space::Self);
        }
    }

    #pragma endregion

    #pragma region Webs

    constexpr float WebLayoutInterval = 0.5f;
    constexpr float WebOutsideCorner = 0.5f;
    const Color WebColor = Color(0.86f, 0.86f, 0.92f, 0.6f);

    UI::Image* AddWeb(HMUI::Screen* screen, bool rightCorner, float size) {
        auto sprite = DanceEventAssets::GetSprite("web.png", {0, 1});
        if (!sprite) return nullptr;
        auto parentCanvas = screen->GetComponent<Canvas*>();
        auto go = GameObject::New_ctor("DanceEventWeb");
        go->set_layer(screen->get_gameObject()->get_layer());
        go->get_transform()->SetParent(screen->get_transform(), false);
        state->screenDecor.emplace_back(go);

        auto canvas = go->AddComponent<Canvas*>();
        canvas->set_overrideSorting(true);
        canvas->set_sortingOrder((parentCanvas ? parentCanvas->get_sortingOrder() : 0) + 10);
        AddShaderChannels(canvas, AdditionalCanvasShaderChannels::TexCoord1.value__ | ImageViewCurveChannel);
        auto rect = go->GetComponent<RectTransform*>();
        rect->set_anchorMin({rightCorner ? 1.0f : 0.0f, 1});
        rect->set_anchorMax({rightCorner ? 1.0f : 0.0f, 1});
        rect->set_pivot({0, 1});
        rect->set_sizeDelta({size, size});
        rect->set_localScale(FastVector3(rightCorner ? -1.0f : 1.0f, 1, 1));

        auto image = AddImage(go->get_transform(), "Web", sprite, BSML::Helpers::GetUINoGlowMat());
        Stretch(image->get_rectTransform());
        image->set_color(WebColor);
        state->webs.push_back({screen, canvas, rect, rightCorner});
        return image;
    }

    void AddSpider(UI::Image* web) {
        auto sprite = DanceEventAssets::GetSprite("spider.png", {0.5f, 0.8f});
        if (!sprite) return;
        auto material = BSML::Helpers::GetUINoGlowMat();
        auto thread = AddImage(web->get_transform(), "Thread", DanceEventAssets::Thread(), material);
        thread->set_color(WithAlpha(WebColor, WebColor.a * 0.5f));
        auto threadRect = thread->get_rectTransform();
        threadRect->set_anchorMin({0.4f, 0.6f});
        threadRect->set_anchorMax({0.4f, 0.6f});
        threadRect->set_pivot({0.5f, 1});
        threadRect->set_sizeDelta({0.25f, 15});
        state->thread = threadRect;

        auto spider = AddImage(threadRect, "Spider", sprite, material);
        auto rect = spider->get_rectTransform();
        rect->set_anchorMin({0.5f, 0});
        rect->set_anchorMax({0.5f, 0});
        rect->set_pivot({0.5f, 0.8f});
        rect->set_sizeDelta({12, 12});
    }

    void SpawnWebs(Random& random, HMUI::ScreenSystem* system) {
        if (!system) return;
        auto left = AddWeb(system->get_leftScreen(), false, Range(random, 20, 26));
        auto right = AddWeb(system->get_rightScreen(), true, Range(random, 20, 26));
        AddWeb(system->get_mainScreen(), Next(random, 0, 2) == 0, Range(random, 14, 18));
        auto spiderWeb = Next(random, 0, 2) == 0 ? left : right;
        if (spiderWeb) AddSpider(spiderWeb);
        state->threadPhase = Range(random, 0, 100);
    }

    bool TryGetVisibleBounds(HMUI::Screen* screen, FastVector2& min, FastVector2& max) {
        min = FastVector2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        max = FastVector2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
        auto corners = Corners();
        auto screenTransform = screen->get_transform();
        auto images = screen->GetComponentsInChildren<UI::Image*>(false);
        for (auto image : images) {
            if (!image->get_enabled() || image->get_color().a < 0.05f) continue;
            if (image->get_canvasRenderer()->GetInheritedAlpha() < 0.05f) continue;
            bool isWeb = std::ranges::any_of(state->webs, [&](Web& web) {
                return web.rect && image->get_transform()->IsChildOf(web.rect.ptr());
            });
            if (isWeb) continue;
            image->get_rectTransform()->GetWorldCorners(corners);
            for (auto corner : corners) {
                auto local = screenTransform->InverseTransformPoint(corner);
                min = FastVector2(std::min(min.x, local.x), std::min(min.y, local.y));
                max = FastVector2(std::max(max.x, local.x), std::max(max.y, local.y));
            }
        }
        return max.x > min.x;
    }

    void LayoutWebs() {
        for (auto& web : state->webs) {
            if (!web.canvas) continue;
            FastVector2 min, max;
            if (!web.screen || !web.rect || !TryGetVisibleBounds(web.screen.ptr(), min, max)) {
                web.canvas->set_enabled(false);
                continue;
            }
            web.canvas->set_enabled(true);
            FastVector2 corner(web.rightCorner ? max.x : min.x, max.y);
            web.rect->set_localPosition(FastVector3(corner.x + (web.rightCorner ? WebOutsideCorner : -WebOutsideCorner), corner.y + WebOutsideCorner, 0));
        }
    }

    void UpdateWebs(float time) {
        if (!MainMenuShown()) {
            if (state->websShown) {
                for (auto& web : state->webs) {
                    if (web.canvas) web.canvas->set_enabled(false);
                }
            }
            state->websShown = false;
            return;
        }
        if (!state->websShown || time >= state->nextWebLayout) {
            LayoutWebs();
            state->nextWebLayout = time + WebLayoutInterval;
            state->websShown = true;
        }
        if (!state->thread) return;
        float t = time + state->threadPhase;
        float length = 10 + 14 * (0.5f + 0.5f * std::sin(t * 0.3f));
        state->thread->set_sizeDelta({0.25f, length});
        state->thread->set_localRotation(Quaternion::Euler(0, 0, 3 * std::sin(t * 0.9f)));
    }

    #pragma endregion

    #pragma region Skeleton

    const FastVector3 SkeletonBackContact = FastVector3(0.422f, 1.153f, -0.313f);

    Material* SkeletonMaterial(std::string const& name) {
        if (name == "bone") return DanceEventModels::Solid(DanceEventModels::Hex("EFE6D2"), 0.35f);
        if (name == "eye_sockets") return DanceEventModels::Solid(DanceEventModels::Hex("140C0A"), 0);
        if (name == "hat") return DanceEventModels::Solid(DanceEventModels::Hex("151515"), 0.1f);
        if (name == "hat_band") return DanceEventModels::Solid(DanceEventModels::Hex("FF6A00"), 0.3f);
        return DanceEventModels::Glowing(DanceEventModels::Hex("FF8A1A"));
    }

    void SpawnSkeleton() {
        auto model = DanceEventModels::Get("skeleton_leaner.obj.gz");
        if (!model) return;
        std::vector<Material*> materials;
        for (auto const& name : model->materials) {
            auto material = SkeletonMaterial(name);
            if (!material) return;
            materials.push_back(material);
        }
        auto go = CreateModel("DanceEventSkeleton", *model, materials);
        go->SetActive(false);
        state->skeleton = go;
    }

    FastVector3 OnCurvedScreen(FastVector3 flat) {
        if (!state->skeletonCurve || state->skeletonCurve->get_radius() <= 0) return flat;
        float radius = state->skeletonCurve->get_radius();
        auto canvas = state->skeletonCurve->get_transform();
        auto local = canvas->InverseTransformPoint(flat);
        float angle = local.x / radius;
        FastVector3 bent(radius * std::sin(angle), local.y, local.z - radius * (1 - std::cos(angle)));
        return canvas->TransformPoint(bent);
    }

    void UpdateSkeleton(FastVector3 head) {
        if (!state->skeleton) return;
        auto panel = BeatLeader::DanceEventPanel::Shown();
        bool show = panel && MainMenuShown();
        if (state->skeleton->get_activeSelf() != show) state->skeleton->SetActive(show);
        if (!show) return;

        if (!state->skeletonPanel || state->skeletonPanel.ptr() != panel) {
            state->skeletonPanel = panel;
            state->skeletonCurve = panel->GetComponentInParent<HMUI::CurvedCanvasSettings*>();
        }
        auto corners = Corners();
        panel->GetWorldCorners(corners);
        auto bottom = OnCurvedScreen(corners[BottomRightCorner]);
        auto top = OnCurvedScreen(corners[TopRightCorner]);
        float along = std::abs(top.y - bottom.y) > 0.0001f ? (SkeletonBackContact.y - bottom.y) / (top.y - bottom.y) : 0;
        auto edge = FastVector3::LerpUnclamped(bottom, top, Clamp01(along));
        FastVector3 toHead = head - edge;
        toHead.y = 0;
        if (toHead.sqrMagnitude() < 0.01f) return;
        auto rotation = FastQuaternion(Quaternion::LookRotation(toHead));
        FastVector3 position = edge - rotation * SkeletonBackContact;
        position.y = 0;
        state->skeleton->get_transform()->SetPositionAndRotation(position, rotation);
    }

    #pragma endregion

    #pragma region Bats

    const FastVector3 DefaultLogoPosition = FastVector3(0, 19, 26);

    void SpawnBats(Random& random) {
        auto batUp = DanceEventAssets::GetSprite("bat-up.png");
        auto batDown = DanceEventAssets::GetSprite("bat-down.png");
        if (!batUp || !batDown) return;
        state->batUp = batUp;
        state->batDown = batDown;
        auto logoObject = GameObject::Find("Logo");
        FastVector3 logo = logoObject ? FastVector3(logoObject->get_transform()->get_position()) : DefaultLogoPosition;
        int count = Next(random, 3, 6);
        for (int i = 0; i < count; i++) {
            float width = Range(random, 1.5f, 2.4f);
            GameObject* root;
            auto image = CreateWorldImage("Bat", batUp, FastVector2(width, width * 0.625f), BSML::Helpers::GetUINoGlowMat(), root);
            root->get_transform()->SetParent(state->root->get_transform(), false);
            Bat bat {
                .root = root->get_transform().ptr(),
                .image = image,
                .center = logo + FastVector3(Range(random, -3, 3), Range(random, -4, 3), 0),
                .radius = Range(random, 6, 11),
                .speed = Range(random, 12, 24) * (Next(random, 0, 2) == 0 ? -1.0f : 1.0f),
                .angle = Range(random, 0, 360),
                .flap = Range(random, 4, 6),
                .phase = Range(random, 0, 10)
            };
            state->bats.push_back(std::move(bat));
        }
    }

    void UpdateBats(FastVector3 head, float time, float deltaTime) {
        if (!state->batUp || !state->batDown) return;
        for (auto& bat : state->bats) {
            if (!bat.root || !bat.image) continue;
            bat.angle += bat.speed * deltaTime;
            float angle = bat.angle * Deg2Rad;
            float t = time + bat.phase;
            float wingBeat = std::sin(t * bat.flap * std::numbers::pi_v<float> * 2);
            FastVector3 offset(std::cos(angle) * bat.radius, 0.4f * wingBeat + 1.2f * std::sin(t * 0.4f), std::sin(angle) * bat.radius * 0.35f);
            FastVector3 position = bat.center + offset;
            bat.root->set_position(position);
            FastVector3 away = position - head;
            if (away.sqrMagnitude() > 0.01f) bat.root->set_rotation(Quaternion::LookRotation(away, FastVector3(0, 1, 0)));
            bool up = wingBeat > 0;
            if (up != bat.wingsUp) {
                bat.wingsUp = up;
                bat.image->set_sprite(up ? state->batUp.ptr() : state->batDown.ptr());
            }
        }
    }

    #pragma endregion

    #pragma region Lifecycle

    void Despawn() {
        if (!state) return;
        if (state->root) UnityEngine::Object::Destroy(state->root.ptr());
        for (auto& decor : state->screenDecor) {
            if (decor) UnityEngine::Object::Destroy(decor.ptr());
        }
        state.reset();
    }

    void Spawn(BeatLeader::DanceEventMenuDecor* owner) {
        if (state) return;
        state = std::make_unique<DecorState>();
        auto root = GameObject::New_ctor("DanceEventMenuDecor");
        root->get_transform()->SetParent(owner->get_transform(), false);
        state->root = root;

        Random random(std::random_device{}());
        std::vector<Spot> taken;
        try {
            SpawnCandles(random, taken);
            SpawnPumpkins(random, taken);
            SpawnMist(random);
            HMUI::ScreenSystem* system = nullptr;
            if (mainMenuView && mainMenuView->get_screen()) {
                system = mainMenuView->get_screen()->GetComponentInParent<HMUI::ScreenSystem*>();
            }
            SpawnWebs(random, system);
            SpawnBats(random);
            SpawnSkeleton();
        } catch (std::exception const& e) {
            BeatLeaderLogger.error("Dance event decor failed: {}", e.what());
        }
        BeatLeaderLogger.info("Dance event decor: {} pumpkins, {} candles, {} webs, {} bats, skeleton {}",
            state->pumpkins.size(), state->candles.size(), state->webs.size(), state->bats.size(), (bool)state->skeleton);
    }

    void Refresh(BeatLeader::DanceEventMenuDecor* decor) {
        if (DanceEvent::Current()) {
            Spawn(decor);
        } else {
            Despawn();
        }
    }

    void HandleDanceEventChanged() {
        if (decorInstance) Refresh(decorInstance.ptr());
    }

    #pragma endregion
}

namespace BeatLeader {
    void DanceEventMenuDecor::Start() {
        static bool listening = false;
        if (!listening) {
            listening = true;
            DanceEvent::AddCurrentChangedListener(HandleDanceEventChanged);
        }
        Refresh(this);
    }

    void DanceEventMenuDecor::Update() {
        if (!state) return;
        if (!state->camera) {
            if (auto camera = (Camera*)Camera::get_main()) state->camera = camera;
        }
        FastVector3 head = state->camera ? FastVector3(state->camera->get_transform()->get_position()) : DefaultHead;
        float time = Time::get_time();
        float deltaTime = Time::get_deltaTime();
        UpdatePumpkins(head, time);
        UpdateCandles(head, time);
        UpdateMist(deltaTime);
        UpdateWebs(time);
        UpdateBats(head, time, deltaTime);
        UpdateSkeleton(head);
    }

    void DanceEventMenuDecor::OnDestroy() {
        Despawn();
    }
}

MAKE_HOOK_MATCH(DanceEventMainMenuDidActivate, &GlobalNamespace::MainMenuViewController::DidActivate, void,
    GlobalNamespace::MainMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    DanceEventMainMenuDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    mainMenuView = self;
    DanceEvent::Load();
    if (!decorInstance) {
        auto go = GameObject::New_ctor("BeatLeaderDanceEventMenuDecor");
        SceneManagement::SceneManager::MoveGameObjectToScene(go, self->get_gameObject()->get_scene());
        decorInstance = go->AddComponent<BeatLeader::DanceEventMenuDecor*>();
    }
}

void BeatLeader::DanceEventMenuDecor::setup() {
    INSTALL_HOOK(BeatLeaderLogger, DanceEventMainMenuDidActivate);
}
