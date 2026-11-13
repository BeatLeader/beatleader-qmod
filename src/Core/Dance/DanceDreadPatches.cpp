#include "include/Core/Dance/DanceDread.hpp"
#include "include/Core/Dance/DanceDreadParts.hpp"

#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "GlobalNamespace/LevelFailedText.hpp"
#include "GlobalNamespace/LevelFailedTextEffect.hpp"
#include "GlobalNamespace/LightWithIdManager.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "GlobalNamespace/TrailElementCollection.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/TubeBloomPrePassLightWithId.hpp"
#include "TMPro/TMP_Text.hpp"
#include "UnityEngine/FindObjectsInactive.hpp"
#include "UnityEngine/FindObjectsSortMode.hpp"

#include "main.hpp"

#include <array>
#include <optional>
#include <unordered_set>

using namespace GlobalNamespace;
using UnityEngine::Color;

#pragma region Lights

namespace DanceDread::Lights {
    static constexpr Color BloodRed = {0.9f, 0.04f, 0.03f, 1};

    struct ManagerColors {
        LightWithIdManager* key;
        safe_ptr<LightWithIdManager*> manager;
        std::array<std::optional<Color>, LightWithIdManager::kMaxLightId + 1> colors;
    };

    struct StaticLight {
        safe_ptr<TubeBloomPrePassLight*> light;
        Color color;
    };

    static bool caching = false;
    static bool refreshing = false;
    static std::vector<ManagerColors> rawColors;
    static std::vector<StaticLight> staticLights;

    static float dim = 0;
    static float tint = 0;
    static float desaturation = 0;

    static Color Recolor(Color color) {
        if (dim <= 0 && tint <= 0 && desaturation <= 0) return color;
        float gray = color.r * 0.3f + color.g * 0.59f + color.b * 0.11f;
        Color faded = {
            color.r + (gray - color.r) * desaturation,
            color.g + (gray - color.g) * desaturation,
            color.b + (gray - color.b) * desaturation,
            color.a
        };
        float brightness = std::max(faded.r, std::max(faded.g, faded.b));
        float keep = 1 - dim;
        return {
            (faded.r + (BloodRed.r * brightness - faded.r) * tint) * keep,
            (faded.g + (BloodRed.g * brightness - faded.g) * tint) * keep,
            (faded.b + (BloodRed.b * brightness - faded.b) * tint) * keep,
            color.a
        };
    }

    static ManagerColors& ColorsOf(LightWithIdManager* manager) {
        for (auto& entry : rawColors) {
            if (entry.key == manager) return entry;
        }
        return rawColors.emplace_back(ManagerColors{manager, manager});
    }

    void BeginLevel() {
        Reset();
        caching = true;
    }

    void CollectStaticLights() {
        staticLights.clear();
        std::unordered_set<TubeBloomPrePassLight*> driven;
        auto withIds = UnityEngine::Object::FindObjectsOfType(i2c::cs_type_of<TubeBloomPrePassLightWithId*>());
        for (auto object : withIds) {
            auto withId = reinterpret_cast<TubeBloomPrePassLightWithId*>(object.unsafe_ptr());
            if (withId->_tubeBloomPrePassLight) driven.insert(withId->_tubeBloomPrePassLight.unsafe_ptr());
        }
        auto lights = UnityEngine::Object::FindObjectsOfType(i2c::cs_type_of<TubeBloomPrePassLight*>());
        for (auto object : lights) {
            auto light = reinterpret_cast<TubeBloomPrePassLight*>(object.unsafe_ptr());
            if (!driven.contains(light)) staticLights.push_back({light, light->color});
        }
        BeatLeaderLogger.info("Dance or Die: {} lights with ids, {} static lights", withIds.size(), staticLights.size());
    }

    static void Refresh() {
        for (auto& [light, color] : staticLights) {
            if (light) light->color = Recolor(color);
        }
        refreshing = true;
        for (auto& entry : rawColors) {
            if (!entry.manager) continue;
            auto manager = entry.manager.ptr();
            for (int id = 0; id < (int)entry.colors.size(); id++) {
                if (auto& color = entry.colors[id]) manager->SetColorForId(id, *color);
            }
        }
        refreshing = false;
    }

    void Apply(float newDim, float newTint, float newDesaturation) {
        if (std::abs(newDim - dim) < 0.004f && std::abs(newTint - tint) < 0.004f && std::abs(newDesaturation - desaturation) < 0.004f) return;
        dim = newDim;
        tint = newTint;
        desaturation = newDesaturation;
        Refresh();
    }

    void Reset() {
        caching = false;
        bool changed = dim > 0 || tint > 0 || desaturation > 0;
        dim = 0;
        tint = 0;
        desaturation = 0;
        if (changed) Refresh();
        rawColors.clear();
        staticLights.clear();
    }

    static void Remember(LightWithIdManager* manager, int lightId, Color color) {
        if (refreshing || lightId < 0 || lightId > LightWithIdManager::kMaxLightId) return;
        ColorsOf(manager).colors[lightId] = color;
    }
}

MAKE_HOOK_MATCH(DanceDreadSetColorForId, &LightWithIdManager::SetColorForId, void, LightWithIdManager* self, int32_t lightId, Color color) {
    if (DanceDread::Lights::caching) {
        DanceDread::Lights::Remember(self, lightId, color);
        color = DanceDread::Lights::Recolor(color);
    }
    DanceDreadSetColorForId(self, lightId, color);
}

#pragma endregion

#pragma region Notes

MAKE_HOOK_MATCH(DanceDreadColorForType, static_cast<Color (ColorManager::*)(ColorType)>(&ColorManager::ColorForType), Color, ColorManager* self, ColorType type) {
    auto result = DanceDreadColorForType(self, type);
    float gray = DanceDread::NoteGray;
    float dim = DanceDread::NoteDim;
    if (gray <= 0 && dim <= 0) return result;
    float luminance = 0.299f * result.r + 0.587f * result.g + 0.114f * result.b;
    float keep = 1 - dim;
    return {
        (result.r + (luminance - result.r) * gray) * keep,
        (result.g + (luminance - result.g) * gray) * keep,
        (result.b + (luminance - result.b) * gray) * keep,
        result.a
    };
}

#pragma endregion

#pragma region Saber trails

MAKE_HOOK_MATCH(DanceDreadTrailUpdateVertices, &SaberTrailRenderer::UpdateVertices, void, SaberTrailRenderer* self, TrailElementCollection* trailElementCollection, Color color) {
    DanceDreadTrailUpdateVertices(self, trailElementCollection, color);
    float droop = DanceDread::TrailDroop;
    if (droop <= 0) return;
    auto vertices = self->_vertices;
    auto uvs = self->_uvs;
    if (!vertices || !uvs) return;
    auto count = std::min(vertices.size(), uvs.size());
    for (size_t i = 0; i < count; i++) {
        float age = uvs[i].y;
        vertices[i].y -= droop * age * age;
    }
}

#pragma endregion

#pragma region Fail text

static constexpr std::string_view DeathText = "YOU DIED\n<size=40%>dance next time</size>";

static void OverrideFailText(LevelFailedText* failedText) {
    if (!failedText) return;
    if (failedText->_text) failedText->_text->richText = true;
    failedText->OverrideText(std::string(DeathText));
}

MAKE_HOOK_MATCH(DanceDreadShowFailEffect, &LevelFailedTextEffect::ShowEffect, void, LevelFailedTextEffect* self) {
    DanceDreadShowFailEffect(self);
    if (!DanceDread::IsHaunting()) return;
    try {
        OverrideFailText(self->GetComponentInChildren<LevelFailedText*>(true));
    } catch (std::exception const& e) {
        BeatLeaderLogger.error("Dance or Die: can't change the fail text: {}", e.what());
    }
}

void DanceDread::ShowDeathText() {
    auto texts = UnityEngine::Object::FindObjectsByType(i2c::cs_type_of<LevelFailedText*>(), UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);
    for (auto text : texts) {
        OverrideFailText(reinterpret_cast<LevelFailedText*>(text.unsafe_ptr()));
    }
}

#pragma endregion

void DanceDread::setup() {
    INSTALL_HOOK(BeatLeaderLogger, DanceDreadSetColorForId);
    INSTALL_HOOK(BeatLeaderLogger, DanceDreadColorForType);
    INSTALL_HOOK(BeatLeaderLogger, DanceDreadTrailUpdateVertices);
    INSTALL_HOOK(BeatLeaderLogger, DanceDreadShowFailEffect);
}
