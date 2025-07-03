#include "include/UI/LeaderboardUI.hpp"

#include "UI/ExperienceBar.hpp"
#include "Utils/ReplayManager.hpp"
#include "shared/Models/Replay.hpp"
#include "shared/Models/Score.hpp"
#include "shared/Models/ClanScore.hpp"
#include "include/Models/ScoresContext.hpp"

#include "include/API/PlayerController.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/Enhancers/MapEnhancer.hpp"

#include "include/UI/UIUtils.hpp"
#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/UI/VotingButton.hpp"
#include "include/UI/VotingUI.hpp"
#include "include/UI/LinksContainer.hpp"
#include "include/UI/LogoAnimation.hpp"
#include "include/UI/PlayerAvatar.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/RoleColorScheme.hpp"
#include "include/UI/Themes/ThemeUtils.hpp"
#include "include/UI/ResultsViewController.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/CaptorClanUI.hpp"
#include "include/UI/QuestUI.hpp"
#include "include/UI/MultiplayerLeaderboard.hpp"

#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/ModConfig.hpp"

#include "include/Managers/LeaderboardContextsManager.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "HMUI/TableView.hpp"
#include "HMUI/TableCell.hpp"
#include "HMUI/IconSegmentedControl.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"

#include "System/Action.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Application.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"
#include "UnityEngine/Rendering/ShaderPropertyType.hpp"
#include "UnityEngine/ProBuilder/ColorUtility.hpp"
#include "UnityEngine/TextCore/GlyphMetrics.hpp"
#include "UnityEngine/TextCore/GlyphRect.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Texture.hpp"
#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LocalLeaderboardViewController.hpp"
#include "GlobalNamespace/LoadingControl.hpp"
#include "GlobalNamespace/LeaderboardTableView.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/LeaderboardTableCell.hpp"
#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"
#include "GlobalNamespace/SwitchSettingsController.hpp"
#include "GlobalNamespace/MainSettingsMenuViewControllersInstaller.hpp"
#include "GlobalNamespace/MultiplayerLobbyController.hpp"
#include "BGLib/Polyglot/LocalizedTextMeshProUGUI.hpp"

#include "TMPro/TMP_Sprite.hpp"
#include "TMPro/TMP_SpriteGlyph.hpp"
#include "TMPro/TMP_SpriteCharacter.hpp"
#include "TMPro/TMP_SpriteAsset.hpp"
#include "TMPro/TMP_FontAssetUtilities.hpp"
#include "TMPro/ShaderUtilities.hpp"

#include "custom-types/shared/delegate.hpp"

#include "main.hpp"

#include <regex>
#include <map>
#include <tuple>
#include <chrono>
#include <thread>
#include <future>

using namespace GlobalNamespace;
using namespace HMUI;
using namespace BSML;
using namespace BeatLeader;
using namespace std;
using UnityEngine::Resources;

namespace LeaderboardUI {
    function<void()> retryCallback;
    PlatformLeaderboardViewController* plvc = NULL;

    TMPro::TextMeshProUGUI* uploadStatus = NULL;

    TMPro::TextMeshProUGUI* playerName = NULL;
    HMUI::ImageView* prestigeIcon = NULL;
    BeatLeader::ExperienceBar* experienceBar = NULL;
    BeatLeader::PlayerAvatar* playerAvatar = NULL;

    UnityEngine::UI::Toggle* showBeatLeaderButton = NULL;
    HMUI::CurvedTextMeshPro* showBeatleaderText = NULL;

    TMPro::TextMeshProUGUI* globalRank = NULL;
    HMUI::ImageView* globalRankIcon = NULL;
    TMPro::TextMeshProUGUI* countryRankAndPp = NULL;
    HMUI::ImageView* countryRankIcon = NULL;
    
    UnityEngine::UI::Button* retryButton = NULL;
    BeatLeader::LogoAnimation* logoAnimation = NULL;
    
    BSML::ClickableImage* upPageButton = NULL;
    BSML::ClickableImage* downPageButton = NULL;
    BSML::ClickableImage* contextsButton = NULL;
    BeatLeader::VotingButton* votingButton = NULL;
    HMUI::HoverHint* contextsButtonHover;
    UnityEngine::GameObject* parentScreen = NULL;

    TMPro::TextMeshProUGUI* loginPrompt = NULL;
    UnityEngine::UI::Button* preferencesButton = NULL;
    QuestUI::CustomTextSegmentedControlData* groupsSelector = NULL;

    BeatLeader::ScoreDetailsPopup* scoreDetailsUI = NULL;
    BeatLeader::RankVotingPopup* votingUI = NULL;
    BeatLeader::LinksContainerPopup* linkContainer = NULL;
    HMUI::ModalView* settingsContainer = NULL;
    HMUI::ModalView* contextsContainer = NULL;
    bool visible = false;
    bool hooksInstalled = false;
    int cachedSelector = -1;
     bool leaderboardLoaded = false;
    int page = 1;
    bool showRetryButton = false;
    int selectedScore = 11;
    static vector<Score> scoreVector = vector<Score>(11);
    static vector<ClanScore> clanScoreVector = vector<ClanScore>(11);

    map<LeaderboardTableCell*, HMUI::ImageView*> avatars;
    map<LeaderboardTableCell*, HMUI::ImageView*> cellBackgrounds;
    map<LeaderboardTableCell*, BSML::ClickableImage*> cellHighlights;
    map<LeaderboardTableCell*, Score> cellScores;
    map<string, int> imageRows;

    static UnityEngine::Color underlineHoverColor = UnityEngine::Color(1.0, 0.0, 0.0, 1.0);

    static UnityEngine::Color ownScoreColor = UnityEngine::Color(0.7, 0.0, 0.7, 0.3);
    static UnityEngine::Color someoneElseScoreColor = UnityEngine::Color(0.07, 0.0, 0.14, 0.05);

    static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0, 0.4, 1.0, 1.0);
    static UnityEngine::Color FadedColor = UnityEngine::Color(0.8, 0.8, 0.8, 0.2);

    static UnityEngine::Color ContextsColor = UnityEngine::Color(0.0, 0.4, 0.8, 0.6);

    static string lastUrl = "";
    static string lastVotingStatusUrl = "";
    static string votingUrl = "";

    static ReplayUploadStatus cachedStatus;
    static string cachedDescription;
    static float cachedProgress; 
    static bool cachedShowRestart;
    static bool statusWasCached;
    static vector<UnityEngine::Transform*> ssElements;
    static UnityEngine::GameObject* ssLoadingIndicator;
    bool ssInstalled = true;
    bool ssWasOpened = false;
    bool showBeatLeader = false;
    bool restoredFromPreferences = false;

    void updatePlayerRank() {
        // Function to calculate the coloring and add the + sign if positive 
        auto getColoredChange = [](float change){
            bool positive = change > 0;
            return " <color=#" + (std::string)(positive ? "00FF00" : "FF0000") + ">" + (positive ? "+" : "");
        };

        // If we have a player, update the global rank, country rank and pp
        auto const& player = PlayerController::currentPlayer;
        if (player != std::nullopt && player->rank > 0 && globalRank && countryRankAndPp) {
            // Calculate the changes
            int rankChange = player->lastRank - player->rank;
            int countryRankChange = player->lastCountryRank - player->countryRank;
            float ppChange = player->pp - player->lastPP;

            // Actually set the labels
            globalRank->SetText("#" + to_string(player->rank) + (rankChange != 0 ? getColoredChange(rankChange) + to_string(rankChange) : ""), true);
            auto rankText = "#" + to_string(player->countryRank) + " " + (countryRankChange != 0 ? getColoredChange(countryRankChange) + to_string(countryRankChange) : "")
                + "  <color=#B856FF>" + to_string_wprecision(player->pp, 2) + "pp " + (ppChange != 0 ? getColoredChange(ppChange) + to_string_wprecision(ppChange, 2) + "pp" : "");
            countryRankAndPp->SetText(rankText, true);

            if (rankText.length() > 60) {
                countryRankAndPp->set_fontSize(3);
                globalRank->set_fontSize(3);
            } else {
                countryRankAndPp->set_fontSize(4);
                globalRank->set_fontSize(4);
            }
        }
    }

    void refreshGroupsSelector() {
        bool shouldShow = showBeatLeader && plvc && plvc->_scopeSegmentedControl->selectedCellNumber == 2;
        if (groupsSelector) {
            groupsSelector->get_gameObject()->SetActive(shouldShow);
        }
        // if (PlayerController::currentPlayer) {
        //     countryControl->set_icon(BundleLoader::bundle->GetCountryIcon(PlayerController::currentPlayer->country));
        // }
        if (plvc) {
            plvc->_levelStatsView->get_gameObject()->SetActive(!shouldShow);
        }
    }

    void updatePlayerInfoLabel() {
        auto const& player = PlayerController::currentPlayer;
        if (!playerName) return;
        if (player != std::nullopt) {
            if (!player->name.empty()) {

                updatePlayerRank();
                
                playerName->set_alignment(TMPro::TextAlignmentOptions::Center);
                playerName->SetText(FormatUtils::FormatNameWithClans(PlayerController::currentPlayer.value(), 25, false), true);

                auto params = GetAvatarParams(player.value(), false);
                playerAvatar->SetPlayer(player->avatar, params.baseMaterial, params.hueShift, params.saturation);
                auto sprite = BundleLoader::bundle->GetCountryIcon(player->country);
                if (plvc != NULL) {
                    if (countryRankIcon) {
                        countryRankIcon->set_sprite(sprite);
                        RectTransform* rectTransform = countryRankIcon->get_transform().cast<RectTransform>();
                        rectTransform->set_sizeDelta({3.2, sprite->get_bounds().get_size().y * 10});
                    }
                }

                if (groupsSelector) {
                    ArrayW<StringW> values(player->clans.size() + 2);
                    values[0] = u" Friends";
                    values[1] = u" Country";
                    
                    for (size_t i = 0; i < player->clans.size(); ++i) {
                        values[i + 2] = StringW(" " + player->clans[i].tag);
                    }
                    
                    groupsSelector->set_texts(values);
                }

                prestigeIcon->gameObject->SetActive(true);
                switch (player->prestige) {
                    case 1:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon1;
                      break;
                    case 2:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon2;
                      break;
                    case 3:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon3;
                      break;
                    case 4:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon4;
                      break;
                    case 5:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon5;
                      break;
                    case 6:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon6;
                      break;
                    case 7:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon7;
                      break;
                    case 8:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon8;
                      break;
                    case 9:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon9;
                      break;
                    case 10:
                      prestigeIcon->sprite =
                          BundleLoader::bundle->PrestigeIcon10;
                      break;
                    default:
                      prestigeIcon->gameObject->SetActive(false);
                      break;
                    }

                    experienceBar->OnProfileRequestStateChanged(
                        player.value(), ReplayUploadStatus::finished);
            } else {
                playerName->SetText(player->name + ", play something!", true);
            }
        } else {
            globalRank->SetText("#0", true);
            countryRankAndPp->SetText("#0", true);
            playerAvatar->HideImage();
            prestigeIcon->gameObject->SetActive(false);
            if (countryRankIcon) {
                countryRankIcon->set_sprite(BundleLoader::bundle->globeIcon);
            }
            playerName->SetText("", true);
            if (groupsSelector) {
                ArrayW<StringW> values(2);
                values[0] = u" Friends";
                values[1] = u" Country";
                groupsSelector->set_texts(values);
            }
        }
    }

    void toggleGroupsSelector(bool show) {
        if (!show) {
            if (BundleLoader::bundle->IconSegmentedControl == nullptr) return;
            ArrayW<::HMUI::IconSegmentedControl::DataItem*> dataItems = ArrayW<::HMUI::IconSegmentedControl::DataItem*>(4);
            for (int index = 0; index < 3; ++index)
            {
                dataItems[index] = plvc->_scopeSegmentedControl->_dataItems.get(index);
            }
            dataItems[3] = BundleLoader::bundle->IconSegmentedControl;

            plvc->_scopeSegmentedControl->SetData(dataItems);

            BundleLoader::bundle->IconSegmentedControl = nullptr;
        } else {
            if (plvc->_scopeSegmentedControl->_dataItems.size() != 4) return;
            BundleLoader::bundle->IconSegmentedControl = plvc->_scopeSegmentedControl->_dataItems.get(3);
            
            ArrayW<::HMUI::IconSegmentedControl::DataItem*> dataItems = ArrayW<::HMUI::IconSegmentedControl::DataItem*>(3);
            for (int index = 0; index < 3; ++index)
            {
                dataItems[index] = plvc->_scopeSegmentedControl->_dataItems.get(index);
            }

            plvc->_scopeSegmentedControl->SetData(dataItems);
        }
    }

    MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
        LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        if (showBeatLeader && firstActivation) {
            HMUI::ImageView* imageView = self->get_transform()->Find("HeaderPanel")->GetComponentInChildren<HMUI::ImageView*>();
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        }

        plvc = self;

        if (parentScreen != NULL) {
            visible = showBeatLeader;
            parentScreen->SetActive(showBeatLeader);
            if (statusWasCached && showBeatLeader) {
                updateStatus(cachedStatus, cachedDescription, cachedProgress, cachedShowRestart);
            }
            toggleGroupsSelector(showBeatLeader);
        }
    }

    MAKE_HOOK_MATCH(
        SegmentedControlHandleCellSelection, 
        &SegmentedControl::HandleCellSelectionDidChange,
        void, 
        SegmentedControl* self,
        ::HMUI::SelectableCell* selectableCell, 
        ::HMUI::SelectableCell_TransitionType transitionType, 
        ::System::Object* changeOwner) {
        SegmentedControlHandleCellSelection(self, selectableCell, transitionType, changeOwner);

        if (plvc &&
            leaderboardLoaded &&
            self == plvc->_scopeSegmentedControl.ptr()) {
            cachedSelector = -1;
            refreshGroupsSelector();
        }
    }

    MAKE_HOOK_MATCH(LeaderboardDeactivate, &PlatformLeaderboardViewController::DidDeactivate, void, PlatformLeaderboardViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        if (ssInstalled && plvc) {
            cachedSelector = plvc->_scopeSegmentedControl->selectedCellNumber;
            leaderboardLoaded = false;
        }
        
        LeaderboardDeactivate(self, removedFromHierarchy, screenSystemDisabling);
        hidePopups();

        if (parentScreen != NULL) {
            visible = false;
            parentScreen->SetActive(false);
        }
    }

    void updateVotingButton() {
        setVotingButtonsState(0);
        hideVotingUIs();
        if (plvc) {
            auto [hash, difficulty, mode] = getLevelDetails(plvc->_beatmapKey);
            string votingStatusUrl = WebUtils::API_URL + "votestatus/" + hash + "/" + difficulty + "/" + mode;

            lastVotingStatusUrl = votingStatusUrl;
            WebUtils::GetAsync(votingStatusUrl, [votingStatusUrl](long status, string response) {
                if (votingStatusUrl == lastVotingStatusUrl && status == 200) {
                    BSML::MainThreadScheduler::Schedule([response] {
                        setVotingButtonsState(stoi(response));
                    });
                }
            }, [](float progress){});
        }
    }

    void setVotingButtonsState(int state){
        if (votingButton) {
            votingButton->SetState(state);
        }
        if(ResultsView::resultsVotingButton){
            ResultsView::resultsVotingButton->SetState(state);
        }
    }

    tuple<string, string, string> getLevelDetails(BeatmapKey levelData)
    {
        string hash = regex_replace((string)levelData.levelId, basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(levelData.difficulty.value__);
        string mode = (string)levelData.beatmapCharacteristic->serializedName;
        return make_tuple(hash, difficulty, mode);
    }

    string lastLeaderboardId = "";

    void refreshFromTheServerScores() {
        auto [hash, difficulty, mode] = getLevelDetails(plvc->_beatmapKey);
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode + "/" + ScoresContexts::getContextForId(getModConfig().Context.GetValue())->key;

        int selectedCellNumber = cachedSelector != -1 ? cachedSelector : plvc->_scopeSegmentedControl->selectedCellNumber;
        int selectedGroup = groupsSelector->segmentedControl->selectedCellNumber;

        switch (selectedCellNumber)
        {
        case 1:
            url += "/global/around";
            break;
        case 2:
            if (selectedGroup == 0) {
                url += "/friends/page";
            } else if (selectedGroup == 1) {
                url += "/country/page";
            } else {
                url += "/clan_" + PlayerController::currentPlayer->clans[selectedGroup - 2].tag + "/page";
            }
            break;
        
        default:
            url += "/global/page";
            break;
        }

        url += "?page=" + to_string(page) + "&player=" + PlayerController::currentPlayer->id;

        lastUrl = url;
        lastLeaderboardId = "";

        WebUtils::GetAsync(url, [url](long status, string stringResult){
            if (url != lastUrl) return;
            if (!showBeatLeader) return;

            if (status != 200) {
                return;
            }

            BSML::MainThreadScheduler::Schedule([status, stringResult] {
                rapidjson::Document result;
                result.Parse(stringResult.c_str());
                if (result.HasParseError() || !result.HasMember("data")) return;

                auto scores = result["data"].GetArray();

                plvc->_scores->Clear();
                if ((int)scores.Size() == 0) {
                    plvc->_loadingControl->Hide();
                    plvc->_hasScoresData = false;
                    plvc->_loadingControl->ShowText("No scores were found!", true);
                    
                    plvc->_leaderboardTableView->_tableView->SetDataSource(plvc->_leaderboardTableView.cast<HMUI::TableView::IDataSource>(), true);
                    return;
                }

                auto metadata = result["metadata"].GetObject();
                int perPage = metadata["itemsPerPage"].GetInt();
                int pageNum = metadata["page"].GetInt();
                int total = metadata["total"].GetInt();
                int topRank = 0;

                for (int index = 0; index < 10; ++index)
                {
                    if (index < (int)scores.Size())
                    {
                        auto const& score = scores[index];
                        
                        Score currentScore = Score(score);
                        scoreVector[index] = currentScore;

                        if (index == 0) {
                            topRank = currentScore.rank;
                            lastLeaderboardId = currentScore.leaderboardId;
                        }
                        
                        if (currentScore.playerId.compare(PlayerController::currentPlayer->id) == 0) {
                            selectedScore = index;
                        }

                        LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                            currentScore.modifiedScore, 
                            FormatUtils::FormatPlayerScore(currentScore), 
                            currentScore.rank, 
                            false);
                        plvc->_scores->Add(scoreData);
                    }
                }
                plvc->_leaderboardTableView->_rowHeight = 6;
                if (selectedScore > 9 && !result["selection"].IsNull()) {
                    Score currentScore = Score(result["selection"]);

                    LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                            currentScore.modifiedScore, 
                            FormatUtils::FormatPlayerScore(currentScore), 
                            currentScore.rank, 
                            false);
                    
                    if (currentScore.rank > topRank) {
                        plvc->_scores->Add(scoreData);
                        scoreVector[10] = currentScore;
                        selectedScore = 10;
                    } else {
                        for (size_t i = 10; i > 0; i--)
                        {
                            scoreVector[i] = scoreVector[i - 1];
                        }
                        plvc->_scores->Insert(0, scoreData);
                        scoreVector[0] = currentScore;
                        selectedScore = 0;
                    }
                    if (plvc->_scores->get_Count() > 10) {
                        plvc->_leaderboardTableView->_rowHeight = 5.5;
                    }
                }
                    
                plvc->_leaderboardTableView->_scores = plvc->_scores;
                plvc->_leaderboardTableView->_specialScorePos = 12;
                if (upPageButton != NULL) {
                    upPageButton->get_gameObject()->SetActive(pageNum != 1);
                    downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);
                }

                plvc->_loadingControl->Hide();
                plvc->_hasScoresData = true;
                plvc->_leaderboardTableView->_tableView->SetDataSource(plvc->_leaderboardTableView.cast<HMUI::TableView::IDataSource>(), true);
            });
        });
        
        string votingStatusUrl = WebUtils::API_URL + "votestatus/" + hash + "/" + difficulty + "/" + mode;
        votingUrl = WebUtils::API_URL + "vote/" + hash + "/" + difficulty + "/" + mode;
        if (lastVotingStatusUrl != votingStatusUrl) {
            updateVotingButton();
        }

        plvc->_loadingControl->ShowText("Loading", true);
    }

    void refreshFromTheServerClans() {
        auto [hash, difficulty, mode] = getLevelDetails(plvc->_beatmapKey);
        string url = WebUtils::API_URL + "v1/clanScores/" + hash + "/" + difficulty + "/" + mode + "/page";

        url += "?page=" + to_string(page);

        lastUrl = url;

        WebUtils::GetAsync(url, [url](long status, string stringResult){
            if (url != lastUrl) return;
            if (!showBeatLeader) return;

            if (status != 200) {
                return;
            }

            BSML::MainThreadScheduler::Schedule([status, stringResult] {
                rapidjson::Document result;
                result.Parse(stringResult.c_str());
                if (result.HasParseError() || !result.HasMember("data")) return;

                auto scores = result["data"].GetArray();

                plvc->_scores->Clear();
                if ((int)scores.Size() == 0) {
                    plvc->_loadingControl->Hide();
                    plvc->_hasScoresData = false;
                    plvc->_loadingControl->ShowText("No clan rankings were found!", true);
                    
                    plvc->_leaderboardTableView->_tableView->SetDataSource(plvc->_leaderboardTableView.cast<HMUI::TableView::IDataSource>(), true);
                    return;
                }

                auto metadata = result["metadata"].GetObject();
                int perPage = metadata["itemsPerPage"].GetInt();
                int pageNum = metadata["page"].GetInt();
                int total = metadata["total"].GetInt();

                for (int index = 0; index < 10; ++index)
                {
                    if (index < (int)scores.Size())
                    {
                        auto const& score = scores[index];
                        
                        ClanScore currentScore = ClanScore(score);
                        clanScoreVector[index] = currentScore;

                        BeatLeaderLogger.info("ClanScore");
                        LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                            currentScore.modifiedScore, 
                            FormatUtils::FormatClanScore(currentScore), 
                            currentScore.rank, 
                            false);
                        plvc->_scores->Add(scoreData);
                    }
                }

                plvc->_leaderboardTableView->_rowHeight = 6;
                    
                plvc->_leaderboardTableView->_scores = plvc->_scores;
                plvc->_leaderboardTableView->_specialScorePos = 12;

                if (upPageButton != NULL) {
                    upPageButton->get_gameObject()->SetActive(pageNum != 1);
                    downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);
                }

                plvc->_loadingControl->Hide();
                plvc->_hasScoresData = true;
                plvc->_leaderboardTableView->_tableView->SetDataSource(plvc->_leaderboardTableView.cast<HMUI::TableView::IDataSource>(), true);
            });
        });

        plvc->_loadingControl->ShowText("Loading", true);
    }

    void updateModifiersButton() {
        auto currentContext = ScoresContexts::getContextForId(getModConfig().Context.GetValue());
        contextsButtonHover->set_text("Currently selected leaderboard - " + currentContext->name);
        contextsButton->set_sprite(BundleLoader::bundle->generalContextIcon);
        if (currentContext->icon != "") {
            Sprites::get_Icon(currentContext->icon, [](UnityEngine::Sprite* sprite) {
                if (sprite) {
                    contextsButton->set_sprite(sprite);
                }
            });
        }
    }

    void voteCallback(bool voted, bool rankable, float stars, int type) {
        if (voted) {
            setVotingButtonsState(0);
            string rankableString = "?rankability=" + (rankable ? (string)"1.0" : (string)"0.0");
            string starsString = stars > 0 ? "&stars=" + to_string_wprecision(stars, 2) : "";
            string typeString = type > 0 ? "&type=" + to_string(type) : "";
            string currentVotingUrl = votingUrl;
            WebUtils::PostJSONAsync(votingUrl + rankableString + starsString + typeString, "", [currentVotingUrl, rankable, type](long status, string response) {
                if (votingUrl != currentVotingUrl) return;

                BSML::MainThreadScheduler::Schedule([status, response, rankable, type] {
                    if (status == 200) {
                        setVotingButtonsState(stoi(response));
                        LevelInfoUI::addVoteToCurrentLevel(rankable, type);
                    } else {
                        setVotingButtonsState(1);
                    } 
                });
            });
        }

        hideVotingUIs();
    }

    void hideVotingUIs()
    {
        if (votingUI && votingUI->modal) {
            votingUI->modal->Hide(true, nullptr);
        }
        if(ResultsView::votingUI && ResultsView::votingUI->modal){ ResultsView::votingUI->modal->Hide(true, nullptr); }
    }

    static bool isLocal = false;

    void clearTable() {
        selectedScore = 11;
        if (plvc->_leaderboardTableView->_scores != NULL) {
            plvc->_leaderboardTableView->_scores->Clear();
        }
        if (plvc->_leaderboardTableView && plvc->_leaderboardTableView->_tableView) {
            plvc->_leaderboardTableView->_tableView->SetDataSource(plvc->_leaderboardTableView.cast<HMUI::TableView::IDataSource>(), true);
        }
    }

    void refreshFromTheServerCurrent() {
        if (CaptorClanUI::showClanRanking) {
            refreshFromTheServerClans();
        } else {
            refreshFromTheServerScores();
        }
    }

    void PageDown() {
        page++;

        clearTable();
        refreshFromTheServerCurrent();
    }

    void PageUp() {
        page--;

        clearTable();
        refreshFromTheServerCurrent();
    }

    void updateLeaderboard(PlatformLeaderboardViewController* self) {
        clearTable();
        page = 1;
        isLocal = false;

        if (ssInstalled && 
            ((ssElements.size() == 0 && !ssWasOpened) 
            || (ssElements.size() < 10 && ssWasOpened))) {
            for (size_t i = 0; i < ssElements.size(); i++)
            {
                ssElements[i]->get_gameObject()->SetActive(true);
            }
            ssElements = vector<UnityEngine::Transform*>();
            ArrayW<UnityEngine::Transform*> transforms = plvc->get_gameObject()->get_transform()->FindObjectsOfType<UnityEngine::Transform*>();
            for (size_t i = 0; i < transforms.size(); i++)
            {
                auto transform = transforms[i];
                auto name =  transform->get_name();

                bool infoIcon = name == "ScoreSaberClickableImage";
                bool header = (name == "BSMLHorizontalLayoutGroup" || name == "BSMLVerticalLayoutGroup" || name == "SettingsButton") &&
                            transform->get_parent() && (transform->get_parent()->get_name() == "PlatformLeaderboardViewController" || (transform->get_parent()->get_parent() &&
                            transform->get_parent()->get_parent()->get_name() == "PlatformLeaderboardViewController"));
                if (infoIcon || header) {
                    transform->get_gameObject()->SetActive(false);
                    ssElements.push_back(transform);
                }
            }

            ssLoadingIndicator = Resources::FindObjectsOfTypeAll<UnityEngine::GameObject*>()->FirstOrDefault([](auto x) { return x->get_name() == "ScoreSaberLoadingIndicator" && x->get_transform()->get_parent()->get_name() != "BSMLStackLayoutGroup"; });
        }

        if (PlayerController::currentPlayer == std::nullopt) {
            self->_loadingControl->Hide();
            
            if (loginPrompt == NULL) {
                loginPrompt = ::BSML::Lite::CreateText(plvc->get_transform(), "Please log in or sign up in settings to post scores!", {24, 0}, {100, 4});
                preferencesButton = ::BSML::Lite::CreateUIButton(plvc->get_transform(), "Open settings", {65, -50}, [](){
                    UIUtils::OpenSettings();
                });
            }
            loginPrompt->get_gameObject()->SetActive(true);
            preferencesButton->get_gameObject()->SetActive(true);

            return;
        }

        if (loginPrompt != NULL) {
            loginPrompt->get_gameObject()->SetActive(false);
            preferencesButton->get_gameObject()->SetActive(false);
        }

        if (uploadStatus == NULL) {
            auto dataItem = plvc->_scopeSegmentedControl->_dataItems.get(2);
            int selectedCell = plvc->_scopeSegmentedControl->selectedCellNumber;
            auto controlRectTransform = plvc->_scopeSegmentedControl->get_transform().cast<RectTransform>();
            if (showBeatLeader) {
                dataItem->set_hintText("Groups");
                controlRectTransform->set_sizeDelta({10, 20});
            } else {
                dataItem->set_hintText("Friends");
                controlRectTransform->set_sizeDelta({10, 30});
            }
            toggleGroupsSelector(showBeatLeader);
            plvc->_scopeSegmentedControl->ReloadData();
            plvc->_scopeSegmentedControl->SelectCellWithNumber(selectedCell);

            parentScreen = CreateCustomScreen(self, UnityEngine::Vector2(480, 160));
            visible = true;

            BeatLeader::initScoreDetailsPopup(
                &scoreDetailsUI, 
                self->get_transform(),
                []() {
                    plvc->Refresh(true, true);
                });
            BeatLeader::initLinksContainerPopup(&linkContainer, self->get_transform());
            BeatLeader::initVotingPopup(&votingUI, self->get_transform(), voteCallback);

            auto playerAvatarImage = ::BSML::Lite::CreateImage(parentScreen->get_transform(), plvc->_aroundPlayerLeaderboardIcon, UnityEngine::Vector2(180, 51), UnityEngine::Vector2(16, 16));
            playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
            playerAvatar->Init(playerAvatarImage);

            prestigeIcon = BSML::Lite::CreateImage(parentScreen->get_transform(), BundleLoader::bundle->PrestigeIcon1, { 130, 55}, {7, 7});

            playerName = ::BSML::Lite::CreateText(parentScreen->get_transform(), "", UnityEngine::Vector2(142, 55), UnityEngine::Vector2(60, 10));
            playerName->set_fontSize(6);

            EmojiSupport::AddSupport(playerName);

            experienceBar = ExperienceBar::Instantiate<ExperienceBar>(parentScreen->get_transform());
            experienceBar->LocalComponent()->ManualInit(parentScreen->get_transform());
            auto expBarTransform = experienceBar->LocalComponent()->HorizontalLayout->get_transform().cast<RectTransform>();
            expBarTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
            expBarTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
            expBarTransform->set_anchoredPosition({138, 126});

            auto rankLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(parentScreen->get_transform());
            rankLayout->set_spacing(3);
            EnableHorizontalFit(rankLayout);
            auto rectTransform = rankLayout->get_transform().cast<RectTransform>();
            rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchoredPosition({138, 50});

            auto globalLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            globalLayout->set_spacing(1);
            EnableHorizontalFit(globalLayout);
            globalRankIcon = ::BSML::Lite::CreateImage(globalLayout->get_transform(), BundleLoader::bundle->globeIcon);
            globalRank = ::BSML::Lite::CreateText(globalLayout->get_transform(), "");

            auto countryLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            countryLayout->set_spacing(1);
            EnableHorizontalFit(countryLayout);
            countryRankIcon = ::BSML::Lite::CreateImage(countryLayout->get_transform(), BundleLoader::bundle->globeIcon);
            countryRankAndPp = ::BSML::Lite::CreateText(countryLayout->get_transform(), "");
            if (PlayerController::currentPlayer != std::nullopt) {
                updatePlayerInfoLabel();
            }

            auto websiteLink = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, []() {
                linkContainer->modal->Show(true, true, nullptr);
            }, UnityEngine::Vector2(100, 50), UnityEngine::Vector2(16, 16));
            
            logoAnimation = websiteLink->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
            logoAnimation->Init(websiteLink);
            websiteLink->onEnter = [](){ 
                logoAnimation->SetGlowing(true);
            };

            websiteLink->onExit = [](){ 
                logoAnimation->SetGlowing(false);
            };

            retryButton = QuestUI::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(116, 59), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                showRetryButton = false;
                retryCallback();
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

            
            uploadStatus = QuestUI::CreateText(parentScreen->get_transform(), "", {200, 60});
            resize(uploadStatus, 100, 3);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);
            upPageButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->upIcon, [](){
                PageUp();
            }, UnityEngine::Vector2(100, 11), UnityEngine::Vector2(5, 5));
            downPageButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->downIcon, [](){
                PageDown();
            }, UnityEngine::Vector2(100, -13), UnityEngine::Vector2(5, 5));

            UIUtils::CreateRoundRectImage(parentScreen->get_transform(), UnityEngine::Vector2(100, -22), UnityEngine::Vector2(7, 7));
            contextsButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->generalContextIcon, [](){
                contextsContainer->Show(true, true, nullptr);
            }, UnityEngine::Vector2(100, -22), UnityEngine::Vector2(5, 5));
            contextsButton->set_defaultColor(ContextsColor);
            contextsButton->set_highlightColor(SelectedColor);
            // We need to add an empty hover hint, so we can set it later to the correct content depending on the selected context
            contextsButtonHover = ::BSML::Lite::AddHoverHint(contextsButton, "");
            initContextsModal(self->get_transform());
            updateModifiersButton();
            UIUtils::CreateRoundRectImage(parentScreen->get_transform(), UnityEngine::Vector2(100, 20), UnityEngine::Vector2(7, 7));
            auto votingButtonImage = ::BSML::Lite::CreateClickableImage(
                parentScreen->get_transform(), 
                BundleLoader::bundle->modifiersIcon, 
                []() {
                    if (votingButton->state != 2) return;
                    
                    votingUI->reset();
                    votingUI->modal->Show(true, true, nullptr);
                },
                UnityEngine::Vector2(100, 20), 
                UnityEngine::Vector2(4, 4)
            );
            votingButton = websiteLink->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            votingButton->Init(votingButtonImage);

            initSettingsModal(self->get_transform());

            auto settingsButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->settingsIcon, [](){
                settingsContainer->Show(true, true, nullptr);
            }, {180, 36}, {4.5, 4.5});

            settingsButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
            settingsButton->set_defaultColor(FadedColor);
            settingsButton->set_highlightColor(SelectedColor);

            auto leaderboardLinkButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->profileIcon, [](){
                string url = WebUtils::WEB_URL + "leaderboard/global/" + lastLeaderboardId;
                static auto UnityEngine_Application_OpenURL = il2cpp_utils::resolve_icall<void, StringW>("UnityEngine.Application::OpenURL");
                UnityEngine_Application_OpenURL(url);
            }, {174, 36}, {4.5, 4.5});

            leaderboardLinkButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
            leaderboardLinkButton->set_defaultColor(FadedColor);
            leaderboardLinkButton->set_highlightColor(SelectedColor);
            
            CaptorClanUI::initCaptorClan(plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject(), plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject()->GetComponentInChildren<TMPro::TextMeshProUGUI*>());
            CaptorClanUI::showClanRankingCallback = []() {
                clearTable();
                refreshFromTheServerCurrent();
            };

            bool hasClans = PlayerController::currentPlayer && !PlayerController::currentPlayer->clans.empty();
            ArrayW<StringW> values(hasClans ? PlayerController::currentPlayer->clans.size() + 2 : 2);
            values[0] = u" Friends";
            values[1] = u" Country";
            if (hasClans) {
                for (size_t i = 0; i < PlayerController::currentPlayer->clans.size(); ++i) {
                    values[i + 2] = StringW(" " + PlayerController::currentPlayer->clans[i].tag);
                }
            }
            groupsSelector = QuestUI::CreateTextSegmentedControl(parentScreen->get_transform(), {347, -36}, {20, 6}, values, [](int index) {
                clearTable();
                refreshFromTheServerCurrent();
            });
            groupsSelector->get_gameObject()->SetActive(false);
        }

        if (parentScreen != NULL) {
            parentScreen->GetComponent<HMUI::Screen*>()->_rootViewController = self;
        }

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(false);
            downPageButton->get_gameObject()->SetActive(false);
        }

        refreshFromTheServerCurrent();

        if (ssLoadingIndicator != nullptr && showBeatLeader) {
            ssLoadingIndicator->SetActive(false);
        }
    }

    Score detailsTextWorkaround;

    void setTheScoreAgain() {
        scoreDetailsUI->setScore(detailsTextWorkaround);
    }

    void updateSelectedLeaderboard() {
        if (loginPrompt && PlayerController::currentPlayer == std::nullopt) {
            loginPrompt->get_gameObject()->SetActive(showBeatLeader);
            preferencesButton->get_gameObject()->SetActive(showBeatLeader);
        }

        LevelInfoUI::SetLevelInfoActive(showBeatLeader);
        ModifiersUI::SetModifiersActive(showBeatLeader);
        CaptorClanUI::setActive(showBeatLeader && getModConfig().CaptureActive.GetValue());

        for (size_t i = 0; i < ssElements.size(); i++)
        {
            ssElements[i]->get_gameObject()->SetActive(!showBeatLeader);
        }

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(showBeatLeader);
            downPageButton->get_gameObject()->SetActive(showBeatLeader);
        }
        
        HMUI::ImageView* imageView = plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
        
        if (showBeatLeader) {
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        } else {
            imageView->set_color(UnityEngine::Color(0.5,0.5,0.5,1));
            imageView->set_color0(UnityEngine::Color(0.5,0.5,0.5,1));
            imageView->set_color1(UnityEngine::Color(0.5,0.5,0.5,1));
        }

        auto dataItem = plvc->_scopeSegmentedControl->_dataItems.get(2);
        int selectedCell = plvc->_scopeSegmentedControl->selectedCellNumber;
        auto rectTransform = plvc->_scopeSegmentedControl->get_transform().cast<RectTransform>();
        if (showBeatLeader) {
            dataItem->set_hintText("Groups");
            rectTransform->set_sizeDelta({10, 20});
        } else {
            dataItem->set_hintText("Friends");
            rectTransform->set_sizeDelta({10, 30});
        }
        toggleGroupsSelector(showBeatLeader);
        plvc->_scopeSegmentedControl->ReloadData();
        plvc->_scopeSegmentedControl->SelectCellWithNumber(selectedCell);

        if (parentScreen != NULL) {
            parentScreen->get_gameObject()->SetActive(showBeatLeader);
            retryButton->get_gameObject()->SetActive(showBeatLeader && showRetryButton);

            plvc->_leaderboardTableView->_rowHeight = 6;
        }
    }

    void refreshLeaderboardCall() {
        auto self = plvc;
        if (showBeatLeader) {
            updateLeaderboard(self);
        }

        if (showBeatLeaderButton == NULL) {
            auto headerTransform = self->get_gameObject()->get_transform()->Find("HeaderPanel")->get_transform();
            showBeatleaderText = BSML::Lite::CreateText(headerTransform, "BeatLeader", {-34.5, 7}, {16, 1});
            showBeatLeaderButton = CreateToggle(headerTransform, showBeatLeader, {-84.5, 26.5}, [](bool changed){
                showBeatLeader = !showBeatLeader;
                if (ssInstalled) {
                    getModConfig().ShowBeatleader.SetValue(showBeatLeader);
                }
                plvc->Refresh(true, true);
                updateSelectedLeaderboard();
            });

            if (!showBeatLeader) {
                BSML::MainThreadScheduler::Schedule([] {
                    HMUI::ImageView* imageView = plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
                    imageView->set_color(UnityEngine::Color(0.5,0.5,0.5,1));
                    imageView->set_color0(UnityEngine::Color(0.5,0.5,0.5,1));
                    imageView->set_color1(UnityEngine::Color(0.5,0.5,0.5,1));
                });
            }
            
            if (ssInstalled) {
                updateSelectedLeaderboard();
            }
        }
    }

    MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool showLoadingIndicator, bool clear) {
        plvc = self;
        if (!showBeatLeader && (!plvc->_beatmapKey.levelId.starts_with("custom_level") || ssInstalled)) {
            RefreshLeaderboard(self, showLoadingIndicator, clear);
            ssWasOpened = true;
        }

        if (ssInstalled && showBeatLeader && ssElements.size() == 0) {
            std::async(std::launch::async, [] () {
                std::this_thread::sleep_for(std::chrono::seconds{1});
                BSML::MainThreadScheduler::Schedule([] {
                    refreshLeaderboardCall();
                });
            });
        } else {
            refreshLeaderboardCall();
        }

        if (!plvc->_beatmapKey.levelId.starts_with("custom_level")) {
            if (showBeatLeaderButton) {
                showBeatLeaderButton->get_gameObject()->set_active(true);
                showBeatleaderText->get_gameObject()->set_active(true);
            }
        } else if (!ssInstalled) {
            if (!showBeatLeader) {
                showBeatLeader = true;
                updateSelectedLeaderboard();
                updateLeaderboard(plvc);
            }
            if (showBeatLeaderButton) {
                showBeatLeaderButton->set_isOn(true);
                showBeatLeaderButton->get_gameObject()->set_active(false);
                showBeatleaderText->get_gameObject()->set_active(false);
            }
        }

        leaderboardLoaded = true;
    }

    MAKE_HOOK_MATCH(RefreshLevelStats, &PlatformLeaderboardViewController::RefreshLevelStats, void, PlatformLeaderboardViewController* self) {
        RefreshLevelStats(self);

        refreshGroupsSelector();
    }

    LeaderboardTableCell* CellForIdxReimplement(LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* leaderboardTableCell = tableView->DequeueReusableCellForIdentifier("Cell").try_cast<LeaderboardTableCell>().value_or(UnityW<LeaderboardTableCell>());
        if (leaderboardTableCell == NULL)
        {
            leaderboardTableCell = (LeaderboardTableCell *)Object::Instantiate<LeaderboardTableCell*>(self->_cellPrefab);
            ((TableCell *)leaderboardTableCell)->set_reuseIdentifier("Cell");
        }
        auto score = self->_scores->get_Item(row);
        leaderboardTableCell->set_rank(score->rank);
        leaderboardTableCell->set_playerName(score->playerName);
        leaderboardTableCell->set_score(score->score);
        leaderboardTableCell->set_showFullCombo(score->fullCombo);
        leaderboardTableCell->set_showSeparator(false);
        leaderboardTableCell->set_specialScore(self->_specialScorePos == row);
        return leaderboardTableCell;
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, UnityW<HMUI::TableCell>, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = showBeatLeader && !isLocal ? CellForIdxReimplement(self, tableView, row) : LeaderboardCellSource(self, tableView, row).cast<LeaderboardTableCell>().ptr();

        if (showBeatLeader && !isLocal) {
            if (result->_playerNameText->get_fontSize() > 3 || result->_playerNameText->get_enableAutoSizing()) {
                result->_playerNameText->set_enableAutoSizing(false);
                result->_playerNameText->set_richText(true);
                
                resize(result->_playerNameText, 24, 0);

                move(result->_rankText, 0.7, 0);

                move(result->_playerNameText, -0.5, 0);
                move(result->_fullComboText, 0.2, 0);
                move(result->_scoreText, 4, 0);
                result->_playerNameText->set_fontSize(3);
                result->_fullComboText->set_fontSize(3);
                result->_scoreText->set_fontSize(2);
                EmojiSupport::AddSupport(result->_playerNameText);

                if (!cellBackgrounds.count(result)) {
                    avatars[result] = ::BSML::Lite::CreateImage(result->get_transform(), plvc->_aroundPlayerLeaderboardIcon, UnityEngine::Vector2(-30.5, 0), UnityEngine::Vector2(4, 4));
                    avatars[result]->get_gameObject()->set_active(getModConfig().AvatarsActive.GetValue());

                    auto scoreSelector = ::BSML::Lite::CreateClickableImage(result->get_transform(), Sprites::get_TransparentPixel(), [result]() {
                        auto openEvent = custom_types::MakeDelegate<System::Action *>((std::function<void()>)setTheScoreAgain);
                        detailsTextWorkaround = cellScores[result];

                        scoreDetailsUI->modal->Show(true, true, openEvent);
                        scoreDetailsUI->setScore(cellScores[result]);
                    }, UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6));
                    
                    scoreSelector->set_material(UnityEngine::Object::Instantiate(BundleLoader::bundle->scoreUnderlineMaterial));
                    
                    cellHighlights[result] = scoreSelector;

                    auto backgroundImage = ::BSML::Lite::CreateImage(result->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6));
                    backgroundImage->set_material(BundleLoader::bundle->scoreBackgroundMaterial);
                    backgroundImage->get_transform()->SetAsFirstSibling();
                    cellBackgrounds[result] = backgroundImage;  

                    // auto tagsList = BSML::Lite::CreateHorizontalLayoutGroup(result->get_transform());
                    // clanGroups[result] = tagsList;         
                }
            }
        } else {
            if (result->_scoreText->get_fontSize() == 2) {
                EmojiSupport::RemoveSupport(result->_playerNameText);
                result->_playerNameText->set_enableAutoSizing(true);
                resize(result->_playerNameText, -24, 0);
                
                move(result->_rankText, -0.7, 0);
                
                move(result->_playerNameText, 0.5, 0);
                move(result->_fullComboText, -0.2, 0);
                move(result->_scoreText, -4, 0);
                result->_playerNameText->set_fontSize(4);
                result->_fullComboText->set_fontSize(4);
                result->_scoreText->set_fontSize(4);
                result->_rankText->set_fontSize(4);
            }
        }
        

        if (!isLocal && showBeatLeader) {
            if (cellBackgrounds.count(result)) {
                if (!CaptorClanUI::showClanRanking) {
                    auto currentScore = scoreVector[row];
                    auto player = currentScore.player;
                    cellBackgrounds[result]->get_gameObject()->set_active(true);

                    result->_playerNameText->GetComponent<UnityEngine::RectTransform*>()->set_anchoredPosition({
                        getModConfig().AvatarsActive.GetValue() ? 10.5f : 6.5f,
                        result->_playerNameText->GetComponent<UnityEngine::RectTransform*>()->get_anchoredPosition().y
                    });
                    avatars[result]->get_gameObject()->set_active(getModConfig().AvatarsActive.GetValue());
                    result->_scoreText->get_gameObject()->set_active(getModConfig().ScoresActive.GetValue());
                    
                    if (row == selectedScore) {
                        cellBackgrounds[result]->set_color(ownScoreColor);
                    } else {
                        cellBackgrounds[result]->set_color(someoneElseScoreColor);
                    }
                    cellScores[result] = currentScore;

                    if (currentScore.rank >= 1000) {
                        result->_rankText->set_fontSize(2);
                    } if (currentScore.rank >= 100) {
                        result->_rankText->set_fontSize(3);
                    } else {
                        result->_rankText->set_fontSize(4);
                    }

                    if (getModConfig().AvatarsActive.GetValue()){
                        avatars[result]->set_sprite(plvc->_aroundPlayerLeaderboardIcon);
                        
                        if (!PlayerController::IsIncognito(player)) {
                            Sprites::get_Icon(player.avatar, [result](UnityEngine::Sprite* sprite) {
                                if (sprite != NULL && avatars[result] != NULL && sprite->get_texture()) {
                                    avatars[result]->set_sprite(sprite);
                                }
                            });
                        }
                    }

                    auto scoreSelector = cellHighlights[result];
                    scoreSelector->get_gameObject()->set_active(true);

                    
                    float hg = idleHighlight(player.role);
                    scoreSelector->set_defaultColor(UnityEngine::Color(hg, 0.0, 0.0, 1.0));
                    scoreSelector->set_highlightColor(underlineHoverColor);
                    schemeForRole(player.role, false).Apply(scoreSelector->get_material());
                } else {
                    auto clan = clanScoreVector[row].clan;
                    cellBackgrounds[result]->get_gameObject()->set_active(true);

                    result->_playerNameText->GetComponent<UnityEngine::RectTransform*>()->set_anchoredPosition({
                        getModConfig().AvatarsActive.GetValue() ? 10.5f : 6.5f,
                        result->_playerNameText->GetComponent<UnityEngine::RectTransform*>()->get_anchoredPosition().y
                    });
                    avatars[result]->get_gameObject()->set_active(getModConfig().AvatarsActive.GetValue());
                    result->_scoreText->get_gameObject()->set_active(getModConfig().ScoresActive.GetValue());
                    
                    if (PlayerController::IsMainClan(clan.tag)) {
                        cellBackgrounds[result]->set_color(ownScoreColor);
                    } else {
                        cellBackgrounds[result]->set_color(someoneElseScoreColor);
                    }

                    if (clan.rank >= 1000) {
                        result->_rankText->set_fontSize(2);
                    } if (clan.rank >= 100) {
                        result->_rankText->set_fontSize(3);
                    } else {
                        result->_rankText->set_fontSize(4);
                    }

                    if (getModConfig().AvatarsActive.GetValue()){
                        avatars[result]->set_sprite(plvc->_aroundPlayerLeaderboardIcon);
                        
                        
                        Sprites::get_Icon(clan.icon, [result](UnityEngine::Sprite* sprite) {
                            if (sprite != NULL && avatars[result] != NULL && sprite->get_texture()) {
                                avatars[result]->set_sprite(sprite);
                            }
                        });
                    }

                    auto scoreSelector = cellHighlights[result];
                    scoreSelector->get_gameObject()->set_active(false);

                    
                    float hg = idleHighlight("");
                    scoreSelector->set_defaultColor(UnityEngine::Color(hg, 0.0, 0.0, 1.0));
                    scoreSelector->set_highlightColor(underlineHoverColor);
                    schemeForRole("", false).Apply(scoreSelector->get_material());
                }
            }
        } else {
            if (cellBackgrounds.count(result) && cellBackgrounds[result]) {
                cellBackgrounds[result]->get_gameObject()->set_active(false);
                avatars[result]->get_gameObject()->set_active(false);
                cellHighlights[result]->get_gameObject()->set_active(false);
            }
        }

        return (TableCell *)result;
    }

    void updateStatus(ReplayUploadStatus status, string description, float progress, bool showRestart) {
        lastVotingStatusUrl = "";

        if (status != ReplayUploadStatus::inProgress) {
            updateVotingButton();
        }
        
        if (visible && showBeatLeader) {
            statusWasCached = false;
            uploadStatus->SetText(description, true);
            switch (status)
            {
                case ReplayUploadStatus::finished:
                    logoAnimation->SetAnimating(false);
                    plvc->HandleDidPressRefreshButton();
                    break;
                case ReplayUploadStatus::error:
                    logoAnimation->SetAnimating(false);
                    if (showRestart) {
                        retryButton->get_gameObject()->SetActive(true);
                        showRetryButton = true;
                    }
                    break;
                case ReplayUploadStatus::inProgress:
                    logoAnimation->SetAnimating(true);
                    if (progress >= 100)
                        uploadStatus->SetText("<color=#b103fcff>Posting replay: Finishing up...", true);
                    break;
            }
        } else {
            statusWasCached = true;
            cachedStatus = status;
            cachedDescription = description;
            cachedProgress = progress;
            cachedShowRestart = showRestart;
        }
    }

    void initContextsModal(UnityEngine::Transform* parent) {
        // Calculate modal height based on number of contexts
        float modalHeight = static_cast<float>((ScoresContexts::allContexts.size() + 1) * 10 + 5);
        auto container = BSML::Lite::CreateModal(parent, {40, modalHeight}, nullptr, true);

        BSML::Lite::CreateText(container->get_transform(), "Scores Context", {-8, 27});

        int i = 0;
        for (const auto& context : ScoresContexts::allContexts) {
            BSML::Lite::CreateUIButton(container->get_transform(), context->name, {20.0f, static_cast<float>(-6 - (i + 1) * 10)}, [context](){
                // Set the new value
                getModConfig().Context.SetValue(context->id);
                // Hide the modal
                contextsContainer->Hide(true, nullptr);
                // Clear the leaderboard
                clearTable();
                // Refresh the context button icon
                updateModifiersButton();
                // Fill the leaderboard
                refreshFromTheServerCurrent();
                // Refresh the player rank
                PlayerController::Refresh(0, [](auto player, auto str){
                    BSML::MainThreadScheduler::Schedule([]{
                        LeaderboardUI::updatePlayerRank();
                    });
                });
            });
            i++;
        }

        contextsContainer = container;
    }

    void initSettingsModal(UnityEngine::Transform* parent){
        auto container = BSML::Lite::CreateModal(parent, {40,60}, nullptr, true);
        
        BSML::Lite::CreateText(container->get_transform(), "Leaderboard Settings", {-13, 27});

        BSML::Lite::CreateText(container->get_transform(), "Avatar", {-14, 18});

        CreateToggle(container->get_transform(), getModConfig().AvatarsActive.GetValue(), {-3, 16}, [](bool value){
            getModConfig().AvatarsActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Clans", {-14, 8});

        CreateToggle(container->get_transform(), getModConfig().ClansActive.GetValue(), {-3, 6}, [](bool value){
            getModConfig().ClansActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Score", {-14, -2});

        CreateToggle(container->get_transform(), getModConfig().ScoresActive.GetValue(), {-3, -4}, [](bool value){
            getModConfig().ScoresActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Time", {-14, -12});

        CreateToggle(container->get_transform(), getModConfig().TimesetActive.GetValue(), {-3, -14}, [](bool value){
            getModConfig().TimesetActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Capture", {-14, -22});

        CreateToggle(container->get_transform(), getModConfig().CaptureActive.GetValue(), {-3, -24}, [](bool value){
            getModConfig().CaptureActive.SetValue(value);
            CaptorClanUI::setActive(value);
            plvc->Refresh(true, true);
        });

        settingsContainer = container;
    }

    UnityEngine::UI::Toggle* CreateToggle(UnityEngine::Transform* parent, bool currentValue, UnityEngine::Vector2 anchoredPosition, std::function<void(bool)> onValueChange)
    {
        // Code adapted from: https://github.com/darknight1050/bsml/blob/master/src/BeatSaberUI.cpp#L826
        static SafePtrUnity<GameObject> toggleCopy;
        if (!toggleCopy) {
            auto foundToggle = Resources::FindObjectsOfTypeAll<UnityEngine::UI::Toggle*>()->FirstOrDefault([](auto x) { return x->get_transform()->get_parent()->get_gameObject()->get_name() == "Fullscreen"; });
            toggleCopy = foundToggle ? foundToggle->get_transform()->get_parent()->get_gameObject() : nullptr;
        }


        GameObject* gameObject = Object::Instantiate(toggleCopy.ptr(), parent, false);
        static ConstString nameTextName("NameText");
        GameObject* nameText = gameObject->get_transform()->Find(nameTextName)->get_gameObject();
        Object::Destroy(gameObject->GetComponent<GlobalNamespace::SwitchSettingsController*>());

        static ConstString name("QuestUICheckboxSetting");
        gameObject->set_name(name);

        gameObject->SetActive(false);

        Object::Destroy(nameText->GetComponent<BGLib::Polyglot::LocalizedTextMeshProUGUI*>());
        nameText->SetActive(false);
        
        UnityEngine::UI::Toggle* toggle = gameObject->GetComponentInChildren<UnityEngine::UI::Toggle*>();
        toggle->set_interactable(true);
        toggle->set_isOn(currentValue);
        toggle->onValueChanged = UnityEngine::UI::Toggle::ToggleEvent::New_ctor();
        if(onValueChange)
            toggle->onValueChanged->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction_1<bool>*>(onValueChange));
        RectTransform* rectTransform = gameObject->GetComponent<RectTransform*>();
        rectTransform->set_anchoredPosition({anchoredPosition.x, anchoredPosition.y - 27});

        gameObject->SetActive(true);
        return toggle;
    }

    MAKE_HOOK_MATCH(LocalLeaderboardDidActivate, &LocalLeaderboardViewController::DidActivate, void, LocalLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        isLocal = true;

        LocalLeaderboardDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    }

    Zenject::DiContainer* mpcontainer;

    MAKE_HOOK_MATCH(MultiplayerLobbyControllerActivateMultiplayerLobby, &MultiplayerLobbyController::ActivateMultiplayerLobby, void, MultiplayerLobbyController* self) {
        MultiplayerLobbyControllerActivateMultiplayerLobby(self);

        MultiplayerLeaderboardManager::Initialize(mpcontainer);
    }
    MAKE_HOOK_MATCH(MultiplayerLobbyControllerDeactivateMultiplayerLobby, &MultiplayerLobbyController::DeactivateMultiplayerLobby, void, MultiplayerLobbyController* self) {
        MultiplayerLobbyControllerDeactivateMultiplayerLobby(self);
        MultiplayerLeaderboardManager::Dispose();
    }

    MAKE_HOOK_MATCH(MainSettingsMenuViewControllersInstallerInstall, &MainSettingsMenuViewControllersInstaller::InstallBindings, void, MainSettingsMenuViewControllersInstaller* installer) {
        MainSettingsMenuViewControllersInstallerInstall(installer);
        mpcontainer = installer->get_Container();
    }

    void setup() {
        if (hooksInstalled) return;

        PlayerController::playerChanged.emplace_back([](std::optional<Player> const& updated) {
            BSML::MainThreadScheduler::Schedule([] {
                if (playerName != NULL) {
                    updatePlayerInfoLabel();
                }
            });
        });

        ssInstalled = false;
        showBeatLeader = true;

        for(auto& modInfo : modloader::get_all())
        {
            if(auto loadedMod = std::get_if<modloader::ModData>(&modInfo))
            {
                if(loadedMod->info.id == "ScoreSaber"){
                    ssInstalled = true;
                    showBeatLeader = getModConfig().ShowBeatleader.GetValue();
                    break;
                }
            }
        }

        INSTALL_HOOK(BeatLeaderLogger, LeaderboardActivate);
        INSTALL_HOOK(BeatLeaderLogger, LeaderboardDeactivate);
        INSTALL_HOOK(BeatLeaderLogger, LocalLeaderboardDidActivate);
        INSTALL_HOOK(BeatLeaderLogger, RefreshLeaderboard);
        INSTALL_HOOK(BeatLeaderLogger, RefreshLevelStats);
        INSTALL_HOOK(BeatLeaderLogger, LeaderboardCellSource);
        INSTALL_HOOK(BeatLeaderLogger, SegmentedControlHandleCellSelection);

        if (!ssInstalled) {
            INSTALL_HOOK(BeatLeaderLogger, MultiplayerLobbyControllerActivateMultiplayerLobby);
            INSTALL_HOOK(BeatLeaderLogger, MultiplayerLobbyControllerDeactivateMultiplayerLobby);
            INSTALL_HOOK(BeatLeaderLogger, MainSettingsMenuViewControllersInstallerInstall);
        }

        ScoresContexts::initializeGeneral();
        LeaderboardContextsManager::UpdateContexts([] {
            if (contextsContainer) {
                initContextsModal(plvc->get_transform());
                updateModifiersButton();
            }
        });

        hooksInstalled = true;
    }

    void reset() {
        uploadStatus = NULL;
        plvc = NULL;
        scoreDetailsUI = NULL;
        votingUI = NULL;
        linkContainer = NULL;
        settingsContainer = NULL;
        contextsContainer = NULL;
        loginPrompt = NULL;
        preferencesButton = NULL;
        parentScreen = NULL;
        cellScores.clear();
        avatars = {};
        cellHighlights = {};
        cellBackgrounds = {};
        showBeatLeaderButton = NULL;
        ResultsView::reset();
        upPageButton = NULL;
        groupsSelector = NULL;
        CaptorClanUI::Reset();
        CaptorClanUI::showClanRanking = false;
        ssWasOpened = false;
        if (ssInstalled) {
            showBeatLeader = getModConfig().ShowBeatleader.GetValue();
        }
        ssElements = {};
        ModifiersUI::ResetModifiersUI();
    }    

    void hidePopups() {
        if (scoreDetailsUI) {
            scoreDetailsUI->modal->Hide(false, nullptr);
        }
        if (votingUI) {
            votingUI->modal->Hide(false, nullptr);
        }
        if (linkContainer) {
            linkContainer->modal->Hide(false, nullptr);
        }
        if (settingsContainer) {
            settingsContainer->Hide(false, nullptr);
        }
        if (contextsContainer) {
            contextsContainer->Hide(false, nullptr);
        }
    }
}