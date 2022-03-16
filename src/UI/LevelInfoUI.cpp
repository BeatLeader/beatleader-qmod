#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/LevelCollectionNavigationController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/StandardLevelDetailViewController.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSegmentedControlController.hpp"
#include "GlobalNamespace/LevelParamsPanel.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/constants.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include <map>
#include <string>
#include <regex>

using namespace GlobalNamespace;
using namespace std;
using namespace QuestUI;
using namespace BeatSaberUI;

TMPro::TextMeshProUGUI* starsLabel = NULL;
TMPro::TextMeshProUGUI* ppLabel = NULL;

HMUI::ImageView* starsImage = NULL;
HMUI::ImageView* ppImage = NULL;

static map<string, float> _mapInfos;
static string selectedMap;

MAKE_HOOK_MATCH(LevelRefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView* self) {
    LevelRefreshContent(self);

    if (starsLabel == NULL) {
        starsLabel = CreateText(self->levelParamsPanel->notesCountText->get_transform(), "", false, UnityEngine::Vector2(24, 3));
        starsImage = CreateImage(self->levelParamsPanel->notesCountText->get_transform(), Sprites::get_StarIcon(), UnityEngine::Vector2(-8.6, 5.6), UnityEngine::Vector2(3, 3));

        ppLabel = CreateText(self->levelParamsPanel->notesPerSecondText->get_transform(), "", false, UnityEngine::Vector2(24, 3));
        ppImage = CreateImage(self->levelParamsPanel->notesPerSecondText->get_transform(), Sprites::get_GraphIcon(), UnityEngine::Vector2(-8.6, 5.6), UnityEngine::Vector2(3, 3));
    }

    string hash = regex_replace(to_utf8(csstrtostr(reinterpret_cast<IPreviewBeatmapLevel*>(self->level)->get_levelID())), basic_regex("custom_level_"), "");
    string difficulty = MapEnhancer::DiffName(self->selectedDifficultyBeatmap->get_difficulty().value);
    string mode = to_utf8(csstrtostr(self->beatmapCharacteristicSegmentedControlController->selectedBeatmapCharacteristic->serializedName));

    string key = hash + difficulty + mode;

    if (_mapInfos.count(key)) {
        starsLabel->SetText(il2cpp_utils::createcsstr(to_string_wprecision(_mapInfos[key], 2)));
        ppLabel->SetText(il2cpp_utils::createcsstr(to_string_wprecision(_mapInfos[key] * 44.0f, 2)));
    } else {
        string url = API_URL + "map/hash/" + hash;

        WebUtils::GetJSONAsync(url, [difficulty, mode, key, hash](long status, bool error, rapidjson::Document& result){
            auto difficulties = result["difficulties"].GetArray();

            for (int index = 0; index < (int)difficulties.Size(); ++index)
            {
                auto value = difficulties[index].GetObject();
                _mapInfos[hash + value["difficultyName"].GetString() + value["modeName"].GetString()] = value["stars"].GetFloat();
            }

            float stars = _mapInfos[key];

            QuestUI::MainThreadScheduler::Schedule([stars] () {
                starsLabel->SetText(il2cpp_utils::createcsstr(to_string_wprecision(stars, 2)));
                ppLabel->SetText(il2cpp_utils::createcsstr(to_string_wprecision(stars * 44.0f, 2)));
            });
        });
    }
}

void SetupLevelInfoUI() {
    LoggerContextObject logger = getLogger().WithContext("load");

    INSTALL_HOOK(logger, LevelRefreshContent);
}