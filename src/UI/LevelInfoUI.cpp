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
#include "include/Models/Song.hpp"
#include "include/Models/Difficulty.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include <numeric>
#include <map>
#include <string>
#include <regex>

using namespace GlobalNamespace;
using namespace std;
using namespace QuestUI;
using namespace BeatSaberUI;

namespace LevelInfoUI {
    TMPro::TextMeshProUGUI* starsLabel = NULL;
    TMPro::TextMeshProUGUI* ppLabel = NULL;
    TMPro::TextMeshProUGUI* typeLabel = NULL;
    TMPro::TextMeshProUGUI* statusLabel = NULL;


    HMUI::ImageView* starsImage = NULL;
    HMUI::ImageView* ppImage = NULL;
    HMUI::ImageView* typeImage = NULL;
    HMUI::ImageView* statusImage = NULL;

    TMPro::TextMeshProUGUI* noSubmissionLabel = NULL;

    static map<string, Song> _mapInfos;
    static map<int, string> mapTypes = {
        {1, "acc"},
        {2, "tech"},
        {4, "midspeed"},
        {8, "speed"}
    };
    static map<int, string> mapStatuses = {
        {0, "unranked"},
        {1, "nominated"},
        {2, "qualified"},
        {3, "ranked"},
        {4, "unrankable"},
        {5, "outdated"}
    };
    static string selectedMap;
    static pair<string, string> lastKey;

    MAKE_HOOK_MATCH(LevelRefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView* self) {
        LevelRefreshContent(self);

        if (self->level == NULL || 
            self->level->get_beatmapLevelData() == NULL || 
            self->beatmapCharacteristicSegmentedControlController == NULL ||
            self->beatmapCharacteristicSegmentedControlController->selectedBeatmapCharacteristic == NULL) return;
        if (starsLabel == NULL) {
            starsLabel = CreateText(self->levelParamsPanel->get_transform(), "0.00", true, UnityEngine::Vector2(-27, 6), UnityEngine::Vector2(8, 4));
            starsLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            starsLabel->set_fontStyle(TMPro::FontStyles::Italic);
            AddHoverHint(starsLabel, "BeatLeader ranked stars");

            starsImage = CreateImage(self->levelParamsPanel->get_transform(), Sprites::get_StarIcon(), UnityEngine::Vector2(-33, 5.6), UnityEngine::Vector2(3, 3));

            ppLabel = CreateText(self->levelParamsPanel->get_transform(), "0", true, UnityEngine::Vector2(-9, 6),  UnityEngine::Vector2(8, 4));
            ppLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            ppLabel->set_fontStyle(TMPro::FontStyles::Italic);
            AddHoverHint(ppLabel, "BeatLeader approximate pp");
            
            ppImage = CreateImage(self->levelParamsPanel->get_transform(), Sprites::get_GraphIcon(), UnityEngine::Vector2(-15.5, 5.6), UnityEngine::Vector2(3, 3));

            typeLabel = CreateText(self->levelParamsPanel->get_transform(), "-", {9, 6}, {8,4});
            typeLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            typeLabel->set_fontStyle(TMPro::FontStyles::Italic);
            AddHoverHint(typeLabel, "Map type\n\nunknown");

            typeImage = CreateImage(self->levelParamsPanel->get_transform(), Sprites::get_ArrowIcon(), {2.5, 5.6}, {3,3});

            statusLabel = CreateText(self->levelParamsPanel->get_transform(), "unr.", {27, 6}, {8,4});
            statusLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            statusLabel->set_fontStyle(TMPro::FontStyles::Italic);
            AddHoverHint(statusLabel, "Ranking status - unranked");

            statusImage = CreateImage(self->levelParamsPanel->get_transform(), Sprites::get_ClipboardIcon(), {20.5, 5.6}, {3,3});

            noSubmissionLabel = CreateText(self->levelParamsPanel->get_transform(), "", true, UnityEngine::Vector2(-5, -20));
            noSubmissionLabel->set_color(UnityEngine::Color(1.0, 0.0, 0.0, 1));
            noSubmissionLabel->set_fontSize(3);
            AddHoverHint(noSubmissionLabel, "Check their settings for 'force' or 'hitbox'");
        }

        bool uploadEnabled = UploadEnabled();
        noSubmissionLabel->get_gameObject()->SetActive(!uploadEnabled);
        if (!uploadEnabled) {
            noSubmissionLabel->set_text(UploadDisablers());
            noSubmissionLabel->set_alignment(TMPro::TextAlignmentOptions::Center);
        }

        // TODO: Why not just substr str.substr("custom_level_".size())
        // essentially, remove prefix
        string hash = regex_replace((string)reinterpret_cast<IPreviewBeatmapLevel*>(self->level)->get_levelID(), basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(self->selectedDifficultyBeatmap->get_difficulty().value);
        string mode = (string)self->beatmapCharacteristicSegmentedControlController->selectedBeatmapCharacteristic->serializedName;

        pair<string, string> key = {hash, difficulty + mode};

        // If we didnt change the level, we can just stay where we already are
        if (lastKey == key) return;

        lastKey = key;
        if (_mapInfos.contains(key.first)) {
            setLabels(_mapInfos[key.first].difficulties[key.second]);
        } else {
            string url = WebUtils::API_URL + "map/modinterface/" + key.first;

            WebUtils::GetJSONAsync(url, [key](long status, bool error, rapidjson::Document const& result){
                // if our request was not successfull, we cant do anything
                if (lastKey == key && status != 200 || error) return;

                auto song = Song(result);

                // We can only add it, if the song is actually valid
                if(song.difficulties.empty()) return;

                // Cache the song and get the selected difficulty
                _mapInfos.insert({key.first, song});
                Difficulty selectedDifficulty = _mapInfos[key.first].difficulties[key.second];

                QuestUI::MainThreadScheduler::Schedule([selectedDifficulty] () {
                    setLabels(selectedDifficulty);
                });
            });
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, LevelRefreshContent);
    }

    void resetStars() {
        _mapInfos = {};
    }

    void addVoteToCurrentLevel(bool rankable, int type) {
        // Edit Value in Cache and set labels to the new values
        if(_mapInfos.contains(lastKey.first)){
            auto diff = &_mapInfos[lastKey.first].difficulties[lastKey.second];
            
            diff->type += type;
            diff->votes.push_back(rankable ? 1 : 0);
            setLabels(*diff);
        }
    }

    void reset() {
        starsLabel = NULL;
    }

    void setLabels(Difficulty selectedDifficulty)
    {
        // Set the Stars & PP Label
        float stars = selectedDifficulty.stars;
        starsLabel->SetText(to_string_wprecision(stars, 2));
        ppLabel->SetText(to_string_wprecision(stars * 51.0f, 2));

        // Create a list of all song types, that are definied for this sond
        vector<string> typeStrings;
        int type = selectedDifficulty.type;
        for (const auto& possibleType : mapTypes)
        {
            if ((possibleType.first & type) == possibleType.first)
            {
                typeStrings.push_back(possibleType.second);
            }
        }

        // Then we create content for the type label
        string typeToSet;
        string typeHoverHint = "Map types\n\n";
        if (typeStrings.size() == 0)
        {
            // No type has been votes at this time -> unknown
            typeToSet = "-";
            typeHoverHint += "unknown";
        }
        else if (typeStrings.size() == 1)
        {
            // If just one type has been voted, then we just set this type in the label & Hovertext
            typeToSet = typeStrings[0];
            // Shorten just midspeed cause of limited space
            if (typeToSet == "midspeed")
            {
                typeToSet = "m.speed";
            }
            typeHoverHint += typeStrings[0];
        }
        else
        {
            // If a song has multiple types set, then we just show the user, that there are more than one and add them all to the hover text
            typeToSet = "mul.";
            for (const string &partMapType : typeStrings)
            {
                typeHoverHint += partMapType + "\n";
            }
            // Remove last newline
            typeHoverHint.pop_back();
        }
        // Actually set the labels with the prepared strings
        typeLabel->SetText(typeToSet);
        AddHoverHint(typeLabel, typeHoverHint);

        string rankingStatus = mapStatuses[selectedDifficulty.status];
        // For better readability show 4 characters for ranked(rank.) and unrankable(unra.)
        int shortWritingChars = (selectedDifficulty.status == 3 || selectedDifficulty.status == 4) ? 4 : 3;

        // Calculate voteRatio from votes
        float voteRatio = 0;
        if (!selectedDifficulty.votes.empty())
        {
            float const count = static_cast<float>(selectedDifficulty.votes.size());
            voteRatio = reduce(selectedDifficulty.votes.begin(), selectedDifficulty.votes.end()) / count;
        }

        // Set Color according to calculated VoteRatio (0% = red, 100% = green)
        statusLabel->SetText(rankingStatus.substr(0, shortWritingChars) + ".");
        statusLabel->set_color(UnityEngine::Color(1-voteRatio,voteRatio,0,1));

        // Set Hovertext with percentage value
        voteRatio *= 100;
        AddHoverHint(statusLabel, "Ranking status - " + rankingStatus + "\n Vote ratio - " + to_string(static_cast<int>(voteRatio)) + "%\nVotes - " + to_string(selectedDifficulty.votes.size()));
    }
}