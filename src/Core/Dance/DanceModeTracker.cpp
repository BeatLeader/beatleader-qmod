#include "include/Utils/ArrayUtils.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "include/Core/Dance/DanceModeTracker.hpp"
#include "include/Core/Dance/DanceDread.hpp"
#include "include/Core/CustomModifiers.hpp"

#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/rapidjson.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapCharacteristic.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/ScoreUIController.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"

#include "main.hpp"

#include <exception>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace GlobalNamespace;

namespace DanceModeTracker {
    static std::unique_ptr<Dance::DanceScorer> scorer;
    static std::optional<Dance::DanceResult> lastResult;
    static bool ranThisLevel = false;
    static bool draining = false;
    static float lastSongTime = 0;

    static safe_ptr<AudioTimeSyncController*> timeSyncController;
    static safe_ptr<GameEnergyCounter*> energyCounter;
    static safe_ptr<TMPro::TextMeshProUGUI*> levelText;
    static float hudSongTime = -1;

    static float FromMappingExtensions(float value) {
        if (value >= 1000) return (value - 1000) / 1000;
        if (value <= -1000) return (value + 1000) / 1000;
        return value;
    }

    // Must match Mathf.LerpUnclamped bit for bit, so no fused multiply-add
    static float LerpUnclamped(float a, float b, float t) {
        #pragma clang fp contract(off)
        return a + (b - a) * t;
    }

    // Memory layout of CustomJSONData's JSONWrapper, read without depending on CustomJSONData
    struct JSONWrapperLayout : Il2CppObject {
        std::optional<std::reference_wrapper<const rapidjson::Value>> value;
    };

    static rapidjson::Value const* GetCustomData(BeatmapDataItem* item) {
        static std::unordered_map<Il2CppClass*, FieldInfo*> fields;
        auto klass = i2c::functions::object_get_class(reinterpret_cast<Il2CppObject*>(item));
        auto it = fields.find(klass);
        if (it == fields.end()) {
            it = fields.emplace(klass, i2c::functions::class_get_field_from_name(klass, "customData")).first;
        }
        if (!it->second) return nullptr;
        Il2CppObject* wrapper = nullptr;
        i2c::functions::field_get_value(reinterpret_cast<Il2CppObject*>(item), it->second, &wrapper);
        if (!wrapper) return nullptr;
        auto& value = reinterpret_cast<JSONWrapperLayout*>(wrapper)->value;
        if (!value || !value->get().IsObject()) return nullptr;
        return &value->get();
    }

    static bool Is(rapidjson::Value const& customData, const char* key, bool value) {
        auto it = customData.FindMember(key);
        return it != customData.MemberEnd() && it->value.IsBool() && it->value.GetBool() == value;
    }

    static bool IsFake(rapidjson::Value const* customData) {
        return customData && (Is(*customData, "NE_fake", true) || Is(*customData, "_fake", true));
    }

    static bool IsUninteractable(rapidjson::Value const& customData) {
        return Is(customData, "uninteractable", true) || Is(customData, "_interactable", false);
    }

    static std::pair<std::optional<float>, std::optional<float>> GetPoint(rapidjson::Value const& customData, const char* key, const char* v2Key) {
        auto it = customData.FindMember(key);
        if (it == customData.MemberEnd()) it = customData.FindMember(v2Key);
        if (it == customData.MemberEnd() || !it->value.IsArray() || it->value.Size() < 2) return {};
        auto const& point = it->value;
        return {
            point[0].IsNumber() ? std::optional(point[0].GetFloat()) : std::nullopt,
            point[1].IsNumber() ? std::optional(point[1].GetFloat()) : std::nullopt
        };
    }

    static std::optional<Dance::DanceWall> GetWall(ObstacleData* wall) {
        float x = FromMappingExtensions(wall->lineIndex);
        float y = (int)wall->lineLayer;
        float width = FromMappingExtensions(wall->width);
        float height = wall->height;

        if (auto customData = GetCustomData(wall)) {
            if (IsFake(customData) || IsUninteractable(*customData)) return std::nullopt;
            auto [px, py] = GetPoint(*customData, "coordinates", "_position");
            if (px && py) {
                x = *px + 2;
                y = *py;
            }
            auto [sw, sh] = GetPoint(*customData, "size", "_scale");
            width = sw.value_or(width);
            height = sh.value_or(height);
        }
        if (wall->duration < 0 || width <= 0 || height <= 0) return std::nullopt;
        return Dance::DanceWall::FromGrid(wall->time, wall->duration, x, y, width, height);
    }

    static void CollectBeatmap(IReadonlyBeatmapData* beatmapData, std::vector<float>& noteTimes, std::vector<Dance::DanceWall>& walls) {
        auto items = beatmapData->allBeatmapDataItems;
        if (!items) return;
        for (auto node = items->head; node; node = node->next) {
            auto item = node->item;
            if (auto note = i2c::try_cast<NoteData*>(item)) {
                if (note->gameplayType != NoteData::GameplayType::Bomb && !IsFake(GetCustomData(item))) {
                    noteTimes.push_back(note->time);
                }
            } else if (auto slider = i2c::try_cast<SliderData*>(item)) {
                auto chain = slider;
                if (chain->sliderType == SliderData::Type::Burst && !IsFake(GetCustomData(item))) {
                    for (int i = 1; i < chain->sliceCount; i++) {
                        noteTimes.push_back(LerpUnclamped(chain->time, chain->tailTime, (float)i / (chain->sliceCount - 1)));
                    }
                }
            } else if (auto obstacle = i2c::try_cast<ObstacleData*>(item)) {
                if (auto wall = GetWall(obstacle)) {
                    walls.push_back(*wall);
                }
            }
            if (node->next == items->head) break;
        }
    }

    void Start(Zenject::DiContainer* container, GameplayCoreSceneSetupData* setupData) {
        Stop();
        lastResult = std::nullopt;
        ranThisLevel = false;
        DanceDread::Prepare(container, setupData);

        if (!CustomModifiers::IsActive(DanceModifier::Id)) return;
        if (NegativeModifiers::HasNegativeModifiers(DanceModifier::Id)) {
            BeatLeaderLogger.warn("Dance Mode can't be combined with negative modifiers, playing without it");
            return;
        }
        if (!setupData || setupData->beatmapKey.characteristic != BeatmapCharacteristic::Standard) {
            BeatLeaderLogger.info("Dance Mode only scores Standard maps");
            return;
        }

        auto beatmapData = reinterpret_cast<IReadonlyBeatmapData*>(container->TryResolve(i2c::cs_type_of<IReadonlyBeatmapData*>()));
        auto initData = reinterpret_cast<AudioTimeSyncController_InitData*>(container->TryResolve(i2c::cs_type_of<AudioTimeSyncController_InitData*>()));
        timeSyncController = reinterpret_cast<AudioTimeSyncController*>(container->TryResolve(i2c::cs_type_of<AudioTimeSyncController*>()));
        if (!beatmapData || !initData || !timeSyncController) {
            BeatLeaderLogger.error("Dance Mode: missing the beatmap or the song timing, playing without it");
            return;
        }

        std::vector<float> noteTimes;
        std::vector<Dance::DanceWall> walls;
        CollectBeatmap(beatmapData, noteTimes, walls);

        float songLength = initData->audioClip ? initData->audioClip->length : 0;
        scorer = std::make_unique<Dance::DanceScorer>(noteTimes, walls, initData->timeScale, songLength);
        lastSongTime = 0;
        ranThisLevel = true;
        BeatLeaderLogger.info("Dance Mode: {} notes, {} walls, speed {}", noteTimes.size(), walls.size(), initData->timeScale);
    }

    void Stop() {
        DanceDread::Stop();
        scorer = nullptr;
        draining = false;
        timeSyncController = nullptr;
        energyCounter = nullptr;
        levelText = nullptr;
        hudSongTime = -1;
    }

    bool RanThisLevel() {
        return ranThisLevel;
    }

    std::optional<Dance::DanceLive> Live() {
        if (!scorer) return std::nullopt;
        return scorer->Live();
    }

    std::optional<Dance::DanceResult> const& LastResult() {
        return lastResult;
    }

    void AddFrame(Frame const& frame) {
        if (!scorer) return;
        auto const& head = frame.head;
        auto const& left = frame.leftHand.position;
        auto const& right = frame.rightHand.position;
        scorer->AddFrame(
            frame.time,
            head.position.x, head.position.y, head.position.z,
            head.rotation.x, head.rotation.y, head.rotation.z, head.rotation.w,
            left.x, left.y, left.z,
            right.x, right.y, right.z
        );
    }

    void AddPause(float songTime) {
        if (scorer) scorer->AddPause(songTime);
    }

    void Finish() {
        if (!scorer) return;
        try {
            lastResult = scorer->Finish();
        } catch (std::exception const& e) {
            BeatLeaderLogger.error("Dance score failed: {}", e.what());
            lastResult = Dance::DanceResult();
        }
        if (lastResult->Valid) {
            BeatLeaderLogger.info("Dance score: {:.4f}, vote share: {:.3f}", lastResult->Score, lastResult->VoteShare);
        } else {
            BeatLeaderLogger.info("Dance score: not enough play to judge");
        }
    }

    static constexpr std::string_view LabelText = "<size=60%>DANCE</size> ";
    static constexpr std::string_view WarningLabelText = "<size=60%><color=#E61A1A>DANCE... OR DIE</color></size> ";
    static constexpr std::string_view DrainColor = "#FF5050";
    static constexpr std::string_view DreadColor = "#FF9A1F";

    static TMPro::TextMeshProUGUI* CreateLevelText(TMPro::TextMeshProUGUI* scoreText) {
        auto go = UnityEngine::GameObject::New_ctor("DanceLevelText");
        auto rect = go->AddComponent<UnityEngine::RectTransform*>();
        rect->SetParent(scoreText->transform->parent, false);
        auto source = scoreText->rectTransform;
        rect->anchorMin = source->anchorMin;
        rect->anchorMax = source->anchorMax;
        rect->pivot = source->pivot;
        rect->anchoredPosition = source->anchoredPosition;
        rect->sizeDelta = source->sizeDelta;
        rect->localScale = source->localScale;

        auto text = go->AddComponent<TMPro::TextMeshProUGUI*>();
        text->font = scoreText->font;
        text->fontSharedMaterial = scoreText->fontSharedMaterial;
        text->fontSize = scoreText->fontSize;
        text->fontStyle = scoreText->fontStyle;
        text->alignment = scoreText->alignment;
        text->color = scoreText->color;
        text->richText = true;
        text->enableWordWrapping = false;
        return text;
    }

    static void RefreshHud(Dance::DanceLive const& live) {
        if (live.SongTime == hudSongTime && levelText) return;
        hudSongTime = live.SongTime;
        if (!levelText) {
            auto scoreUI = BeatLeader::FirstOrDefault(UnityEngine::Resources::FindObjectsOfTypeAll<ScoreUIController*>());
            if (!scoreUI || !scoreUI->_scoreText) return;
            levelText = CreateLevelText(scoreUI->_scoreText);
            scoreUI->_scoreText->enabled = false;
        }
        if (!live.Ready) {
            levelText->text = std::string(LabelText) + "...";
            return;
        }
        auto level = fmt::format("{:.0f}%", live.Level * 100);
        auto label = DanceDread::IsWarning() ? WarningLabelText : LabelText;
        std::optional<std::string_view> color;
        if (live.ShouldDrain) {
            color = DrainColor;
        } else if (DanceDread::IsHaunting() && live.Level < DanceDread::DreadLine) {
            color = DreadColor;
        }
        levelText->text = std::string(label) + (color ? fmt::format("<color={}>{}</color>", *color, level) : level);
    }

    void Tick() {
        if (!scorer || !timeSyncController) return;
        auto songTime = timeSyncController->songTime;
        auto deltaTime = songTime - lastSongTime;
        lastSongTime = songTime;

        auto live = scorer->Live();
        draining = live.ShouldDrain;
        if (draining && deltaTime > 0) {
            if (!energyCounter) {
                energyCounter = BeatLeader::FirstOrDefault(UnityEngine::Resources::FindObjectsOfTypeAll<GameEnergyCounter*>());
            }
            if (energyCounter) {
                energyCounter->_nextFrameEnergyChange -= (float)(Dance::DanceScorer::SuggestedDrainPerSecond * deltaTime);
            }
        }
        DanceDread::Update(live);
        RefreshHud(live);
    }

    MAKE_HOOK_MATCH(DanceEnergyHandleNoteWasCut, &GameEnergyCounter::HandleNoteWasCut, void, GameEnergyCounter* self, NoteController* noteController, by_ref<const NoteCutInfo> noteCutInfo) {
        if (draining && noteCutInfo.as_mutable()->allIsOK && noteController->noteData->gameplayType != NoteData::GameplayType::Bomb) {
            return;
        }
        DanceEnergyHandleNoteWasCut(self, noteController, noteCutInfo);
    }

    MAKE_HOOK_MATCH(DanceScoreUIUpdateScore, static_cast<void (ScoreUIController::*)(int32_t)>(&ScoreUIController::UpdateScore), void, ScoreUIController* self, int32_t displayScore) {
        if (scorer) {
            return;
        }
        DanceScoreUIUpdateScore(self, displayScore);
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, DanceEnergyHandleNoteWasCut);
        INSTALL_HOOK(BeatLeaderLogger, DanceScoreUIUpdateScore);
    }
}
