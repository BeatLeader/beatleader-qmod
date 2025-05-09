#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/LevelCollectionNavigationController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/StandardLevelDetailViewController.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSegmentedControlController.hpp"
#include "GlobalNamespace/LevelParamsPanel.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "shared/Models/Song.hpp"
#include "shared/Models/Difficulty.hpp"
#include "shared/Models/TriangleRating.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/UI/CaptorClanUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "main.hpp"

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "scotland2/shared/modloader.h"

#include <numeric>
#include <map>
#include <string>
#include <regex>

using namespace GlobalNamespace;
using namespace std;
using namespace BSML;
using namespace Lite;

namespace LevelInfoUI {
    TMPro::TextMeshProUGUI* starsLabel = NULL;
    TMPro::TextMeshProUGUI* ppLabel = NULL;
    TMPro::TextMeshProUGUI* typeLabel = NULL;
    TMPro::TextMeshProUGUI* statusLabel = NULL;


    HMUI::ImageView* starsImage = NULL;
    HMUI::ImageView* ppImage = NULL;
    HMUI::ImageView* typeImage = NULL;
    HMUI::ImageView* statusImage = NULL;

    static SafePtrUnity<HMUI::ModalView> skillTriangleContainer = NULL;
    static SafePtrUnity<UnityEngine::Material> skillTriangleMat = NULL;

    TMPro::TextMeshProUGUI* noSubmissionLabel = NULL;

    bool bslInstalled = false;
    bool submissionLabel = false;

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
    
    const Difficulty defaultDiff = Difficulty(0, 0, {}, {}, {}, {});
    static TriangleRating currentlySelectedRating = defaultDiff.rating;

    MAKE_HOOK_MATCH(DidDeactivate, &StandardLevelDetailViewController::DidDeactivate, void, StandardLevelDetailViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);
        // Ensure that the skillTriangle is closed when exiting the LevelDetailView. Otherwise we will get a ghost modal on the next open
        if(skillTriangleContainer)
            skillTriangleContainer->Hide(false, nullptr);
    }

    MAKE_HOOK_MATCH(LevelRefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView* self) {
        LevelRefreshContent(self);

        if (!self->_beatmapLevel || 
            !self->_beatmapCharacteristicSegmentedControlController ||
            !self->_beatmapCharacteristicSegmentedControlController->_selectedBeatmapCharacteristic) return;
        if (starsLabel == NULL) {

            ///////////////////////////
            // Skill Triangle
            ///////////////////////////

            // Create Modal
            skillTriangleContainer = BSML::Lite::CreateModal(self->_levelParamsPanel->get_transform(), {40,40}, nullptr, true);

            // Create Actual Triangle Image
            auto skillTriangleImage = BSML::Lite::CreateImage(skillTriangleContainer->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, {0, 0}, {35, 35});
            skillTriangleMat = UnityEngine::Material::Instantiate(BundleLoader::bundle->skillTriangleMaterial);
            skillTriangleImage->set_material(skillTriangleMat.ptr());
            int normalizedValuesPropertyId = UnityEngine::Shader::PropertyToID("_Normalized");

            // Create Star Value Labels for Triangle
            auto techLabel = BSML::Lite::CreateText(skillTriangleContainer->get_transform(), "Tech - ", {-18, 16});
            auto accLabel = BSML::Lite::CreateText(skillTriangleContainer->get_transform(), "Acc - ", {6, 16});
            auto passLabel = BSML::Lite::CreateText(skillTriangleContainer->get_transform(), "Pass - ", {-7, -12});

            // OnClick Function to open the SkillTriangle
            auto openSkillTriangle = [techLabel, accLabel, passLabel, normalizedValuesPropertyId](){
                if(currentlySelectedRating.stars > 0) {
                    techLabel->SetText("Tech - " + to_string_wprecision(currentlySelectedRating.techRating, 2), true);
                    accLabel->SetText("Acc - " + to_string_wprecision(currentlySelectedRating.accRating, 2), true);
                    passLabel->SetText("Pass - " + to_string_wprecision(currentlySelectedRating.passRating, 2), true);
                    skillTriangleMat->SetVector(normalizedValuesPropertyId, {
                        clamp(currentlySelectedRating.techRating / 15.0f, 0.0f, 1.0f),
                        clamp(currentlySelectedRating.accRating / 15.0f, 0.0f, 1.0f),
                        clamp(currentlySelectedRating.passRating / 15.0f, 0.0f, 1.0f),
                        0.0f
                    });
                    skillTriangleContainer->Show(true, true, NULL);
                }
            };

            ///////////////////////////
            // Init Stars, PP, Type, Status and NoSubmission Label
            ///////////////////////////

            starsLabel = CreateText(self->_levelParamsPanel->get_transform(), "0.00", UnityEngine::Vector2(-27, 6), UnityEngine::Vector2(8, 4));
            starsLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            starsImage = CreateClickableImage(self->_levelParamsPanel->get_transform(), Sprites::get_StarIcon(), openSkillTriangle, UnityEngine::Vector2(-33, 5.6), UnityEngine::Vector2(3, 3));
            AddHoverHint(starsLabel, "Song not ranked");

            ppLabel = CreateText(self->_levelParamsPanel->get_transform(), "0", UnityEngine::Vector2(-9, 6),  UnityEngine::Vector2(8, 4));
            ppLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            AddHoverHint(ppLabel, "BeatLeader approximate pp");
            
            ppImage = CreateImage(self->_levelParamsPanel->get_transform(), Sprites::get_GraphIcon(), UnityEngine::Vector2(-15.5, 5.6), UnityEngine::Vector2(3, 3));

            typeLabel = CreateText(self->_levelParamsPanel->get_transform(), "-", {9, 6}, {8,4});
            typeLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            AddHoverHint(typeLabel, "Map type\n\nunknown");

            typeImage = CreateImage(self->_levelParamsPanel->get_transform(), Sprites::get_ArrowIcon(), {2.5, 5.6}, {3,3});

            statusLabel = CreateText(self->_levelParamsPanel->get_transform(), "unr.", {27, 6}, {8,4});
            statusLabel->set_color(UnityEngine::Color(0.651,0.651,0.651, 1));
            AddHoverHint(statusLabel, "Ranking status - unranked \nTo vote for a song to be ranked, click the message box on the leaderboard");

            statusImage = CreateImage(self->_levelParamsPanel->get_transform(), Sprites::get_ClipboardIcon(), {20.5, 5.6}, {3,3});
        }

        if (!submissionLabel) {
            submissionLabel = true;
            noSubmissionLabel = CreateText(self->_levelParamsPanel->get_transform(), "", true, UnityEngine::Vector2(-5, bslInstalled ? -24 : -20));
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

        // Why not just substr str.substr("custom_level_".size())?
        // Because not every level is a custom level.
        string hash = regex_replace((string)self->_beatmapLevel->levelID, basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(self->beatmapKey.difficulty.value__);
        string mode = (string)self->_beatmapCharacteristicSegmentedControlController->_selectedBeatmapCharacteristic->serializedName;

        pair<string, string> key = {hash, difficulty + mode};

        // If we didnt change the level, we can just stay where we already are
        if (lastKey == key) return;

        lastKey = key;
        if (_mapInfos.contains(key.first)) {
            setLabels(_mapInfos[key.first].difficulties[key.second]);
            CaptorClanUI::setClan(_mapInfos[key.first].difficulties[key.second].clanStatus);
        } else {
            string url = WebUtils::API_URL + "map/modinterface/" + key.first;

            setLabels(Difficulty());
            CaptorClanUI::setClan(ClanRankingStatus());

            WebUtils::GetAsync(url, [key](long status, string stringResult){
                // If the map was already switched again, the response is irrelevant
                if(lastKey != key) return;

                BSML::MainThreadScheduler::Schedule([status, key, stringResult] () {
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

                    CaptorClanUI::setClan(selectedDifficulty.clanStatus);
                });
            });
        }
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, LevelRefreshContent);
        INSTALL_HOOK(BeatLeaderLogger, DidDeactivate);

        for(auto& modInfo : modloader::get_all())
        {
            if(auto loadedMod = std::get_if<modloader::ModData>(&modInfo))
            {
                if(loadedMod->info.id == "BetterSongList"){
                    bslInstalled = true;
                    break;
                }
            }
        }
    }

    void SetLevelInfoActive(bool active) {
        if (starsLabel != NULL) {
            starsLabel->get_gameObject()->SetActive(active);
            starsImage->get_gameObject()->SetActive(active);
            ppLabel->get_gameObject()->SetActive(active);
            ppImage->get_gameObject()->SetActive(active);
            typeLabel->get_gameObject()->SetActive(active);
            typeImage->get_gameObject()->SetActive(active);
            statusLabel->get_gameObject()->SetActive(active);
            statusImage->get_gameObject()->SetActive(active);
        }
        if (submissionLabel) {
            noSubmissionLabel->get_gameObject()->SetActive(active);
        }
        if (skillTriangleContainer) {
            skillTriangleContainer->Hide(false, nullptr);
        }
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
        submissionLabel = false;
    }

    void refreshRatingLabels(){
        // Refresh rating labels from cache
        if(starsLabel && _mapInfos.contains(lastKey.first)){ 
            setRatingLabels(_mapInfos[lastKey.first].difficulties[lastKey.second].rating);
        }
    }

    void setLabels(Difficulty selectedDifficulty)
    {
        if (!starsLabel) return;

        // The difficulty may have changed therefor we need to tell the ModifiersUI the new Values and Ratings
        ModifiersUI::songModifiers = selectedDifficulty.modifierValues;
        ModifiersUI::songModifierRatings = selectedDifficulty.modifiersRating;

        // After that we set the rating labels (stars & pp). This also sets the rating labels on the ModifiersUI
        setRatingLabels(selectedDifficulty.rating);

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
        typeLabel->SetText(typeToSet, true);
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
        statusLabel->SetText(rankingStatus.substr(0, shortWritingChars) + ".", true);
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
    }

    void setRatingLabels(TriangleRating rating) {
        // Save rating so that the triangle window knows which values to show
        currentlySelectedRating = rating;

        // refresh ModifiersRating and get potentially selected rating
        TriangleRating modifierRating = ModifiersUI::refreshAllModifiers();

        // if a modifier rating is selected we want to show that one
        if(ModifiersUI::ModifiersAvailable() && modifierRating.stars > 0)
            currentlySelectedRating = modifierRating;
        
        // Set the stars and pp
        starsLabel->SetText(to_string_wprecision(UIUtils::getStarsToShow(currentlySelectedRating), 2), true);
        ppLabel->SetText(to_string_wprecision(currentlySelectedRating.stars * 51.0f, 2), true);

        // Add Hoverhint with all star ratings
        string starsHoverHint;
        if (currentlySelectedRating.stars)
        {
            starsHoverHint = "Overall - " + to_string_wprecision(currentlySelectedRating.stars, 2) 
            + "\nTech - " + to_string_wprecision(currentlySelectedRating.techRating, 2) 
            + "\nAcc - " + to_string_wprecision(currentlySelectedRating.accRating, 2)
            + "\nPass - " + to_string_wprecision(currentlySelectedRating.passRating, 2);
        }
        else 
        {
            starsHoverHint = "Song not ranked";
        }
        AddHoverHint(starsLabel, starsHoverHint);
    }
}