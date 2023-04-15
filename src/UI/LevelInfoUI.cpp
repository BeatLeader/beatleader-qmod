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
#include "include/Assets/BundleLoader.hpp"
#include "include/Models/Song.hpp"
#include "include/Models/Difficulty.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/ModifiersUI.hpp"
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

    SafePtrUnity<HMUI::ModalView> skillTriangleContainer = NULL;
    SafePtrUnity<UnityEngine::Material> skillTriangleMat = NULL;

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
        {5, "outdated"},
        {6, "inevent"}
    };
    static string selectedMap;
    static pair<string, string> lastKey;

    const Difficulty defaultDiff = Difficulty(0, 0, 0, {}, {}, 0, 0, 0);
    static Difficulty currentlySelectedDiff = defaultDiff;

    MAKE_HOOK_MATCH(LevelRefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView* self) {
        LevelRefreshContent(self);

        if (self->level == NULL || 
            self->level->get_beatmapLevelData() == NULL || 
            self->beatmapCharacteristicSegmentedControlController == NULL ||
            self->beatmapCharacteristicSegmentedControlController->selectedBeatmapCharacteristic == NULL) return;
        if (starsLabel == NULL) {

            ///////////////////////////
            // Skill Triangle
            ///////////////////////////

            // Create Modal
            skillTriangleContainer = QuestUI::BeatSaberUI::CreateModal(self->get_transform(), {40,40}, nullptr, true);

            // Create Actual Triangle Image
            auto skillTriangleImage = QuestUI::BeatSaberUI::CreateImage(skillTriangleContainer->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, {0, 0}, {35, 35});
            skillTriangleMat = UnityEngine::Material::Instantiate(BundleLoader::bundle->skillTriangleMaterial);
            skillTriangleImage->set_material(skillTriangleMat.ptr());
            int normalizedValuesPropertyId = UnityEngine::Shader::PropertyToID("_Normalized");

            // Create Star Value Labels for Triangle
            auto techLabel = QuestUI::BeatSaberUI::CreateText(skillTriangleContainer->get_transform(), "Tech - ", {12, 12});
            auto accLabel = QuestUI::BeatSaberUI::CreateText(skillTriangleContainer->get_transform(), "Acc - ", {34, 12});
            auto passLabel = QuestUI::BeatSaberUI::CreateText(skillTriangleContainer->get_transform(), "Pass - ", {23, -17});

            // OnClick Function to open the SkillTriangle
            auto openSkillTriangle = [techLabel, accLabel, passLabel, normalizedValuesPropertyId](){
                int mapType = currentlySelectedDiff.status;
                if(mapType != 0 && mapType != 4 && mapType != 5) {
                    techLabel->SetText("Tech - " + to_string_wprecision(currentlySelectedDiff.techRating, 2));
                    accLabel->SetText("Acc - " + to_string_wprecision(currentlySelectedDiff.accRating, 2));
                    passLabel->SetText("Pass - " + to_string_wprecision(currentlySelectedDiff.passRating, 2));
                    skillTriangleMat->SetVector(normalizedValuesPropertyId, {
                        clamp(currentlySelectedDiff.techRating / 15.0f, 0.0f, 1.0f),
                        clamp(currentlySelectedDiff.accRating / 15.0f, 0.0f, 1.0f),
                        clamp(currentlySelectedDiff.passRating / 15.0f, 0.0f, 1.0f),
                        0.0f
                    });
                    skillTriangleContainer->Show(true, true, NULL);
                }
            };

            ///////////////////////////
            // Init Stars, PP, Type, Status and NoSubmission Label
            ///////////////////////////

            starsLabel = CreateText(self->levelParamsPanel->get_transform(), "0.00", true, UnityEngine::Vector2(-27, 6), UnityEngine::Vector2(8, 4));
            starsLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            starsLabel->set_fontStyle(TMPro::FontStyles::Italic);
            starsImage = CreateClickableImage(self->levelParamsPanel->get_transform(), Sprites::get_StarIcon(), UnityEngine::Vector2(-33, 5.6), UnityEngine::Vector2(3, 3), openSkillTriangle);

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
            AddHoverHint(statusLabel, "Ranking status - unranked \nTo vote for a song to be ranked, click the message box on the leaderboard");

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

            WebUtils::GetAsync(url, [key](long status, string stringResult){
                // If the map was already switched again, the response is irrelevant
                if(lastKey != key) return;

                QuestUI::MainThreadScheduler::Schedule([status, key, stringResult] () {
                    if (status != 200) {
                        setLabels(defaultDiff);
                        return;
                    }

                    rapidjson::Document result;
                    result.Parse(stringResult.c_str());
                    if (result.HasParseError()) {
                        setLabels(defaultDiff);
                        return;
                    }
                    
                    Difficulty selectedDifficulty;
                    Song song = Song(result);
                    if (!song.difficulties.empty())
                    {
                        // If the request was successful, we cache the value and display it
                        _mapInfos.insert({ key.first, song});
                        selectedDifficulty = _mapInfos[key.first].difficulties[key.second];
                    }
                    else
                    {
                        // If it wasnt we just display our default values
                        selectedDifficulty = defaultDiff;
                    }
                
                    setLabels(selectedDifficulty);
                });
            });
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, LevelRefreshContent);
    }

    void SetLevelInfoActive(bool active) {
        starsLabel->get_gameObject()->SetActive(active);
        starsImage->get_gameObject()->SetActive(active);
        ppLabel->get_gameObject()->SetActive(active);
        ppImage->get_gameObject()->SetActive(active);
        typeLabel->get_gameObject()->SetActive(active);
        typeImage->get_gameObject()->SetActive(active);
        statusLabel->get_gameObject()->SetActive(active);
        statusImage->get_gameObject()->SetActive(active);
        noSubmissionLabel->get_gameObject()->SetActive(active);
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
        currentlySelectedDiff = selectedDifficulty;
        // Find the wanted star value
        float stars;
        switch(getModConfig().StarValueToShow.GetValue()){
            case 1:
                stars = selectedDifficulty.techRating;
                break;
            case 2:
                stars = selectedDifficulty.accRating;
                break;
            case 3:
                stars = selectedDifficulty.passRating;
                break;
            default:
                stars = selectedDifficulty.stars;
                break;
        }
        
        // Set the stars and pp
        starsLabel->SetText(to_string_wprecision(stars, 2));
        ppLabel->SetText(to_string_wprecision(selectedDifficulty.stars * 51.0f, 2));

        // Add Hoverhint with all star ratings
        string starsHoverHint;
        if(stars)
        {
            starsHoverHint = "Overall - " + to_string_wprecision(selectedDifficulty.stars, 2) 
            + "\nTech - " + to_string_wprecision(selectedDifficulty.techRating, 2) 
            + "\nAcc - " + to_string_wprecision(selectedDifficulty.accRating, 2)
            + "\nPass - " + to_string_wprecision(selectedDifficulty.passRating, 2);
        }
        else 
        {
            starsHoverHint = "Song not ranked";
        }
        AddHoverHint(starsLabel, starsHoverHint);

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

        // If the given rankingStatus is unknown, we default to unranked
        if(rankingStatus.empty()){
            rankingStatus = mapStatuses[0];
        }

        // Calculate voteRatio from votes
        float rating = 0;
        float reviewScore = 0;
        if (!selectedDifficulty.votes.empty())
        {
            float count = static_cast<float>(selectedDifficulty.votes.size());
            reviewScore = reduce(selectedDifficulty.votes.begin(), selectedDifficulty.votes.end()) / count;
            rating = reviewScore - (reviewScore - 0.5f) * pow(2.0f, -log10(count + 1));
        }

        // Set Color according to calculated VoteRatio (0% = red, 100% = green)
        statusLabel->SetText(rankingStatus.substr(0, shortWritingChars) + ".");
        if (rating == 0) {
            statusLabel->set_color(UnityEngine::Color(0.5, 0.5, 0.5, 1));
        } else {
            statusLabel->set_color(UnityEngine::Color(1 - rating, rating, 0, 1));
        }
        

        // Set Hovertext with percentage value
        rating *= 100;
        reviewScore *= 100;
        AddHoverHint(statusLabel, "Ranking status - " + rankingStatus 
                                + "\nRating - " + to_string(static_cast<int>(rating))
                                + "%\nPositivity ratio - " + to_string(static_cast<int>(reviewScore)) 
                                + "%\nVotes - " + to_string(selectedDifficulty.votes.size())
                                + "\nTo vote for a song to be ranked, click the message box on the leaderboard");

        // Set the modifier values of this song on the gameplaymodifiers panel
        ModifiersUI::songModifiers = selectedDifficulty.modifierValues;
        ModifiersUI::refreshAllModifiers();
    }
}