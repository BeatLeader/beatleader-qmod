#include "include/UI/LeaderboardUI.hpp"

#include "include/Models/Replay.hpp"
#include "include/Models/Score.hpp"
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

#include "include/UI/LeaderboardViewController.hpp"
#include "include/UI/Leaderboard.hpp"

#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/ModConfig.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "HMUI/TableView.hpp"
#include "HMUI/TableCell.hpp"
#include "HMUI/IconSegmentedControl.hpp"
#include "HMUI/IconSegmentedControl_DataItem.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"

#include "System/Action.hpp"
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
#include "GlobalNamespace/LeaderboardTableView.hpp"
#include "GlobalNamespace/LeaderboardTableView_ScoreData.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel_ScoresScope.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/LeaderboardTableCell.hpp"
#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"

#include "TMPro/TMP_Sprite.hpp"
#include "TMPro/TMP_SpriteGlyph.hpp"
#include "TMPro/TMP_SpriteCharacter.hpp"
#include "TMPro/TMP_SpriteAsset.hpp"
#include "TMPro/TMP_FontAssetUtilities.hpp"
#include "TMPro/ShaderUtilities.hpp"

#include "bsml/shared/BSML.hpp"

#include "custom-types/shared/delegate.hpp"
#include "leaderboardcore/shared/LeaderboardCore.hpp"

#include "main.hpp"

#include <regex>
#include <map>
#include <tuple>
#include <chrono>
#include <thread>
#include <future>

using namespace GlobalNamespace;
using namespace HMUI;
using namespace QuestUI;
using namespace BeatLeader;
using namespace std;
using UnityEngine::Resources;

DEFINE_TYPE(LeaderboardUI, LeaderboardViewController);

namespace LeaderboardUI {
    function<void()> retryCallback;

    BeatLeader::Leaderboard leaderboard = BeatLeader::Leaderboard();
    PlatformLeaderboardViewController* originalplvc = NULL;
    LeaderboardViewController* plvc = NULL;
    GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap;

    TMPro::TextMeshProUGUI* uploadStatus = NULL;

    TMPro::TextMeshProUGUI* playerName = NULL;
    BeatLeader::PlayerAvatar* playerAvatar = NULL;
    UnityEngine::UI::Toggle* showBeatLeaderButton = NULL;

    TMPro::TextMeshProUGUI* globalRank = NULL;
    HMUI::ImageView* globalRankIcon = NULL;
    TMPro::TextMeshProUGUI* countryRankAndPp = NULL;
    HMUI::ImageView* countryRankIcon = NULL;
    
    UnityEngine::UI::Button* retryButton = NULL;
    BeatLeader::LogoAnimation* logoAnimation = NULL;
    
    QuestUI::ClickableImage* upPageButton = NULL;
    QuestUI::ClickableImage* downPageButton = NULL;
    QuestUI::ClickableImage* contextsButton = NULL;
    BeatLeader::VotingButton* votingButton = NULL;
    HMUI::HoverHint* contextsButtonHover;
    UnityEngine::GameObject* parentScreen = NULL;

    TMPro::TextMeshProUGUI* loginPrompt = NULL;
    UnityEngine::UI::Button* preferencesButton = NULL;

    BeatLeader::ScoreDetailsPopup* scoreDetailsUI = NULL;
    BeatLeader::RankVotingPopup* votingUI = NULL;
    BeatLeader::LinksContainerPopup* linkContainer = NULL;
    HMUI::ModalView* settingsContainer = NULL;
    HMUI::ModalView* contextsContainer = NULL;
    bool visible = true;
    bool hooksInstalled = false;
    int page = 1;
    bool showRetryButton = false;
    bool isLocal = false;
    int selectedScore = 11;
    static vector<Score> scoreVector = vector<Score>(11);

    map<LeaderboardTableCell*, HMUI::ImageView*> avatars;
    map<LeaderboardTableCell*, HMUI::ImageView*> cellBackgrounds;
    map<LeaderboardTableCell*, QuestUI::ClickableImage*> cellHighlights;
    map<LeaderboardTableCell*, Score> cellScores;
    map<string, int> imageRows;

    static UnityEngine::Color underlineHoverColor = UnityEngine::Color(1.0, 0.0, 0.0, 1.0);

    static UnityEngine::Color ownScoreColor = UnityEngine::Color(0.7, 0.0, 0.7, 0.3);
    static UnityEngine::Color someoneElseScoreColor = UnityEngine::Color(0.07, 0.0, 0.14, 0.05);

    static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0, 0.4, 1.0, 1.0);
    static UnityEngine::Color FadedColor = UnityEngine::Color(0.8, 0.8, 0.8, 0.2);

    static string lastUrl = "";
    static string lastVotingStatusUrl = "";
    static string votingUrl = "";

    static ReplayUploadStatus cachedStatus;
    static string cachedDescription;
    static float cachedProgress; 
    static bool cachedShowRestart;
    static bool statusWasCached;
    bool showBeatLeader = true;

    static map<Context, string> contextToUrlString = {
        {Context::Standard, "modifiers"},
        {Context::NoMods, "standard"},
        {Context::NoPause, "nopause"},
        {Context::Golf, "golf"},
    };

    static map<Context, string> contextToDisplayString = {
        {Context::Standard, "General"},
        {Context::NoMods, "No Mods"},
        {Context::NoPause, "No Pauses"},
        {Context::Golf, "Golf"},
    };

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
            globalRank->SetText("#" + to_string(player->rank) + (rankChange != 0 ? getColoredChange(rankChange) + to_string(rankChange) : ""));
            countryRankAndPp->SetText("#" + to_string(player->countryRank) + " " + (countryRankChange != 0 ? getColoredChange(countryRankChange) + to_string(countryRankChange) : "")
                + "  <color=#B856FF>" + to_string_wprecision(player->pp, 2) + "pp " + (ppChange != 0 ? getColoredChange(ppChange) + to_string_wprecision(ppChange, 2) + "pp" : ""));
        }
    }

    void updatePlayerInfoLabel() {
        auto const& player = PlayerController::currentPlayer;
        if (player != std::nullopt) {
            if (!player->name.empty()) {

                updatePlayerRank();
                
                playerName->set_alignment(TMPro::TextAlignmentOptions::Center);
                playerName->SetText(FormatUtils::FormatNameWithClans(PlayerController::currentPlayer.value(), 25));

                auto params = GetAvatarParams(player.value(), false);
                playerAvatar->SetPlayer(player->avatar, params.baseMaterial, params.hueShift, params.saturation);
                
                if (plvc != NULL) {
                    auto sprite = BundleLoader::bundle->GetCountryIcon(player->country);
                    auto countryControl = plvc->scopeSegmentedControl->dataItems.get(3);
                    countryControl->set_hintText("Country");

                    plvc->scopeSegmentedControl->dataItems.get(3)->set_icon(sprite);
                    plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);
                    if (countryRankIcon) {
                        countryRankIcon->set_sprite(sprite);
                        RectTransform* rectTransform = (RectTransform*)countryRankIcon->get_transform();
                        rectTransform->set_sizeDelta({3.2, sprite->get_bounds().get_size().y * 10});
                    }
                }

            } else {
                playerName->SetText(player->name + ", play something!");
            }
        } else {
            globalRank->SetText("#0");
            countryRankAndPp->SetText("#0");
            playerAvatar->HideImage();
            if (countryRankIcon) {
                countryRankIcon->set_sprite(BundleLoader::bundle->locationIcon);
            }
            playerName->SetText("");
        }
    }


    void LeaderboardDidActivate() {
        if (plvc && plvc->isActivated) {      
            if (parentScreen != NULL) {
                if (statusWasCached) {
                    updateStatus(cachedStatus, cachedDescription, cachedProgress, cachedShowRestart);
                }
            }
        }
    }

    void LeaderboardDidDeactivate() {
        hidePopups();
    }

    MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
        LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        originalplvc = self;
        
        LeaderboardDidActivate();
    }

    MAKE_HOOK_MATCH(LeaderboardDeactivate, &PlatformLeaderboardViewController::DidDeactivate, void, PlatformLeaderboardViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        LeaderboardDeactivate(self, removedFromHierarchy, screenSystemDisabling);

        LeaderboardDidDeactivate();
    }

    void updateVotingButton() {
        setVotingButtonsState(0);
        hideVotingUIs();
        
        if (plvc && (plvc->isActivated || plvc->wasActivatedBefore)) {
            auto [hash, difficulty, mode] = getLevelDetails(reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level()));
            string votingStatusUrl = WebUtils::API_URL + "votestatus/" + hash + "/" + difficulty + "/" + mode;

            lastVotingStatusUrl = votingStatusUrl;
            WebUtils::GetAsync(votingStatusUrl, [votingStatusUrl](long status, string response) {
                if (votingStatusUrl == lastVotingStatusUrl && status == 200) {
                    QuestUI::MainThreadScheduler::Schedule([response] {
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

    tuple<string, string, string> getLevelDetails(IPreviewBeatmapLevel* levelData)
    {
        string hash = regex_replace((string)levelData->get_levelID(), basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(difficultyBeatmap->get_difficulty().value);
        string mode = (string)difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
        return make_tuple(hash, difficulty, mode);
    }

    void refreshFromTheServer() {
        auto [hash, difficulty, mode] = getLevelDetails(reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level()));
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode + "/" + contextToUrlString[static_cast<Context>(getModConfig().Context.GetValue())];

        switch (plvc->scopeSegmentedControl->selectedCellNumber)
        {
        case 1:
            url += "/global/around";
            break;
        case 2:
            url += "/friends/page";
            break;
        case 3:
            url += "/country/page";
            break;
        
        default:
            url += "/global/page";
            break;
        }

        url += "?page=" + to_string(page) + "&player=" + PlayerController::currentPlayer->id;

        lastUrl = url;

        WebUtils::GetAsync(url, [url](long status, string stringResult){
            if (url != lastUrl) return;
            if (!showBeatLeader) return;

            if (status != 200) {
                return;
            }

            QuestUI::MainThreadScheduler::Schedule([status, stringResult] {
                rapidjson::Document result;
                result.Parse(stringResult.c_str());
                if (result.HasParseError() || !result.HasMember("data")) return;

                auto scores = result["data"].GetArray();

                plvc->leaderboardTableView->scores->Clear();
                if ((int)scores.Size() == 0) {
                    plvc->loadingLabel->SetText("No scores were found!");
                    
                    plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
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
                        }
                        
                        if (currentScore.playerId.compare(PlayerController::currentPlayer->id) == 0) {
                            selectedScore = index;
                        }

                        LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                            currentScore.modifiedScore, 
                            FormatUtils::FormatPlayerScore(currentScore), 
                            currentScore.rank, 
                            false);
                        plvc->leaderboardTableView->scores->Add(scoreData);
                    }
                }
                plvc->leaderboardTableView->rowHeight = 6;
                if (selectedScore > 9 && !result["selection"].IsNull()) {
                    Score currentScore = Score(result["selection"]);

                    LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                            currentScore.modifiedScore, 
                            FormatUtils::FormatPlayerScore(currentScore), 
                            currentScore.rank, 
                            false);
                    
                    if (currentScore.rank > topRank) {
                        plvc->leaderboardTableView->scores->Add(scoreData);
                        scoreVector[10] = currentScore;
                        selectedScore = 10;
                    } else {
                        for (size_t i = 10; i > 0; i--)
                        {
                            scoreVector[i] = scoreVector[i - 1];
                        }
                        plvc->leaderboardTableView->scores->Insert(0, scoreData);
                        scoreVector[0] = currentScore;
                        selectedScore = 0;
                    }
                    if (plvc->leaderboardTableView->scores->get_Count() > 10) {
                        plvc->leaderboardTableView->rowHeight = 5.5;
                    }
                }
                
                plvc->leaderboardTableView->specialScorePos = 12;
                if (upPageButton != NULL) {
                    upPageButton->get_gameObject()->SetActive(pageNum != 1);
                    downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);
                }

                plvc->loadingLabel->SetText("");
                plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
            });
        });
        
        string votingStatusUrl = WebUtils::API_URL + "votestatus/" + hash + "/" + difficulty + "/" + mode;
        votingUrl = WebUtils::API_URL + "vote/" + hash + "/" + difficulty + "/" + mode;
        if (lastVotingStatusUrl != votingStatusUrl) {
            updateVotingButton();
        }

        plvc->loadingLabel->SetText("Loading...");
    }

    void updateModifiersButton() {
        contextsButtonHover->set_text("Currently selected leaderboard - " + contextToDisplayString[static_cast<Context>(getModConfig().Context.GetValue())]);

        Sprite* modifiersIcon = NULL;
        switch(static_cast<Context>(getModConfig().Context.GetValue()))
        {
            case Context::Standard:
                modifiersIcon = BundleLoader::bundle->generalContextIcon;
                break;
            case Context::NoMods:
                modifiersIcon = BundleLoader::bundle->noModifiersIcon;
                break;
            case Context::NoPause:
                modifiersIcon = BundleLoader::bundle->noPauseIcon;
                break;
            case Context::Golf:
                modifiersIcon = BundleLoader::bundle->golfIcon;
                break;
        }

        if(modifiersIcon != NULL)
        {
            contextsButton->set_sprite(modifiersIcon);
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

                QuestUI::MainThreadScheduler::Schedule([status, response, rankable, type] {
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

    void clearTable() {
        selectedScore = 11;

        if (plvc->leaderboardTableView && plvc->leaderboardTableView->scores) {
            plvc->leaderboardTableView->scores->Clear();
        } else {
            plvc->leaderboardTableView->scores = ::System::Collections::Generic::List_1<::GlobalNamespace::LeaderboardTableView::ScoreData*>::New_ctor();
        }

        if (plvc->leaderboardTableView && plvc->leaderboardTableView->tableView) {
            plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
        }
    }

    void PageDown() {
        page++;

        clearTable();
        refreshFromTheServer();
    }

    void PageUp() {
        page--;

        clearTable();
        refreshFromTheServer();
    }

    void updateLeaderboard(LeaderboardViewController* self) {
        clearTable();
        page = 1;

        if (PlayerController::currentPlayer == std::nullopt) {
            plvc->loadingLabel->SetText("");
            
            if (preferencesButton == NULL) {
                loginPrompt = ::QuestUI::BeatSaberUI::CreateText(plvc->get_transform(), "Please sign up or log in to post scores!", false, UnityEngine::Vector2(4, 10));
                preferencesButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->get_transform(), "Open settings", UnityEngine::Vector2(0, 0), [](){
                    UIUtils::OpenSettings();
                });
            }
            loginPrompt->get_gameObject()->SetActive(true);
            preferencesButton->get_gameObject()->SetActive(true);

            return;
        }

        if (preferencesButton != NULL) {
            loginPrompt->get_gameObject()->SetActive(false);
            preferencesButton->get_gameObject()->SetActive(false);
        }

        if (uploadStatus == NULL) {
            parentScreen = CreateCustomScreen(leaderboard.get_panelViewController(), UnityEngine::Vector2(480, 160), leaderboard.get_panelViewController()->get_transform()->get_position(), 0);
            parentScreen->get_transform()->SetParent(leaderboard.get_panelViewController()->get_transform());
            visible = true;

            BeatLeader::initScoreDetailsPopup(
                &scoreDetailsUI, 
                plvc->get_transform(),
                []() {
                    updateLeaderboard(plvc);
                });
            BeatLeader::initLinksContainerPopup(&linkContainer, plvc->get_transform());
            BeatLeader::initVotingPopup(&votingUI, plvc->get_transform(), voteCallback);

            auto playerAvatarImage = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), BundleLoader::bundle->defaultAvatar, UnityEngine::Vector2(180, 51), UnityEngine::Vector2(16, 16));
            playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
            playerAvatar->Init(playerAvatarImage);

            globalRankIcon = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), BundleLoader::bundle->globeIcon, UnityEngine::Vector2(120, 45), UnityEngine::Vector2(4, 4));
            playerName = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(140, 53), UnityEngine::Vector2(60, 10));
            playerName->set_fontSize(6);

            EmojiSupport::AddSupport(playerName);

            auto rankLayout = ::QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(parentScreen->get_transform());
            rankLayout->set_spacing(3);
            EnableHorizontalFit(rankLayout);
            auto rectTransform = (RectTransform*)rankLayout->get_transform();
            rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchoredPosition({138, 45});

            auto globalLayout = ::QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            globalLayout->set_spacing(1);
            EnableHorizontalFit(globalLayout);
            globalRankIcon = ::QuestUI::BeatSaberUI::CreateImage(globalLayout->get_transform(), BundleLoader::bundle->globeIcon);
            globalRank = ::QuestUI::BeatSaberUI::CreateText(globalLayout->get_transform(), "", false);

            auto countryLayout = ::QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            countryLayout->set_spacing(1);
            EnableHorizontalFit(countryLayout);
            countryRankIcon = ::QuestUI::BeatSaberUI::CreateImage(countryLayout->get_transform(), BundleLoader::bundle->globeIcon);
            countryRankAndPp = ::QuestUI::BeatSaberUI::CreateText(countryLayout->get_transform(), "", false);
            if (PlayerController::currentPlayer != std::nullopt) {
                updatePlayerInfoLabel();
            }

            auto websiteLink = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, UnityEngine::Vector2(105, 50), UnityEngine::Vector2(16, 16), []() {
                linkContainer->modal->Show(true, true, nullptr);
            });
            
            logoAnimation = websiteLink->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
            logoAnimation->Init(websiteLink);
            websiteLink->get_onPointerEnterEvent() += [](auto _){ 
                logoAnimation->SetGlowing(true);
            };

            websiteLink->get_onPointerExitEvent() += [](auto _){ 
                logoAnimation->SetGlowing(false);
            };

            if (retryButton) UnityEngine::GameObject::Destroy(retryButton);
            retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(105, 59), UnityEngine::Vector2(15, 7), [](){
                retryButton->get_gameObject()->SetActive(false);
                showRetryButton = false;
                retryCallback();
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

            if(uploadStatus) UnityEngine::GameObject::Destroy(uploadStatus);
            uploadStatus = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false);
            move(uploadStatus, 150, 56);
            resize(uploadStatus, 10, 0);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);
            upPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(plvc->get_transform(), Sprites::get_UpIcon(), UnityEngine::Vector2(-40, 23), UnityEngine::Vector2(8, 5.12), [](){
                PageUp();
            });
            downPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(plvc->get_transform(), Sprites::get_DownIcon(), UnityEngine::Vector2(-40, -13), UnityEngine::Vector2(8, 5.12), [](){
                PageDown();
            });

            contextsButton = ::QuestUI::BeatSaberUI::CreateClickableImage(plvc->get_transform(), BundleLoader::bundle->modifiersIcon, UnityEngine::Vector2(-40, 34), UnityEngine::Vector2(6, 6), [](){
                contextsContainer->Show(true, true, nullptr);
            });
            contextsButton->set_defaultColor(SelectedColor);
            contextsButton->set_highlightColor(SelectedColor);
            // We need to add an empty hover hint, so we can set it later to the correct content depending on the selected context
            contextsButtonHover = ::QuestUI::BeatSaberUI::AddHoverHint(contextsButton, "");
            initContextsModal(self->get_transform());
            updateModifiersButton();

            auto votingButtonImage = ::QuestUI::BeatSaberUI::CreateClickableImage(
                plvc->get_transform(), 
                BundleLoader::bundle->modifiersIcon, 
                UnityEngine::Vector2(-40, 28), 
                UnityEngine::Vector2(4, 4), 
                []() {
                if (votingButton->state != 2) return;
                
                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            votingButton = websiteLink->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            votingButton->Init(votingButtonImage);

            initSettingsModal(plvc->get_transform());

            auto settingsButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->settingsIcon, {180, 36}, {4.5, 4.5}, [](){
                settingsContainer->Show(true, true, nullptr);
            });

            settingsButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
            settingsButton->set_defaultColor(FadedColor);
            settingsButton->set_highlightColor(SelectedColor);
        }

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(false);
            downPageButton->get_gameObject()->SetActive(false);
        }

        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level());
        if (!levelData->get_levelID().starts_with("custom_level")) {
            setVotingButtonsState(-1);
            plvc->loadingLabel->SetText("Leaderboards for this map are not supported!");
            plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
        } else {
            refreshFromTheServer();
        }
    }

    Score detailsTextWorkaround;

    void setTheScoreAgain() {
        scoreDetailsUI->setScore(detailsTextWorkaround);
    }

    void updateSelectedLeaderboard() {
        if (preferencesButton && PlayerController::currentPlayer == std::nullopt) {
            loginPrompt->get_gameObject()->SetActive(showBeatLeader);
            preferencesButton->get_gameObject()->SetActive(showBeatLeader);
        }

        LevelInfoUI::SetLevelInfoActive(showBeatLeader);
        ModifiersUI::SetModifiersActive(showBeatLeader);

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(showBeatLeader);
            downPageButton->get_gameObject()->SetActive(showBeatLeader);
        }

        if (parentScreen != NULL) {
            retryButton->get_gameObject()->SetActive(showRetryButton);

            plvc->leaderboardTableView->rowHeight = 6;
        }
    }

    void Refresh() {
        if (!plvc || (!plvc->isActivated && !plvc->wasActivatedBefore)) return;
        updateLeaderboard(plvc);
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, HMUI::TableCell*, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

        if (plvc == NULL || plvc->leaderboardTableView != self) {
            return result;
        }

        if (showBeatLeader && !isLocal) {
        if (result->playerNameText->get_fontSize() > 3 || result->playerNameText->get_enableAutoSizing()) {
            result->playerNameText->set_enableAutoSizing(false);
            result->playerNameText->set_richText(true);
            
            resize(result->playerNameText, 24, 0);
            move(result->rankText, -6.2, -0.1);
            result->rankText->set_alignment(TMPro::TextAlignmentOptions::Right);

            move(result->playerNameText, -0.5, 0);
            move(result->fullComboText, 0.2, 0);
            move(result->scoreText, 4, 0);
            result->playerNameText->set_fontSize(3);
            result->fullComboText->set_fontSize(3);
            result->scoreText->set_fontSize(2);
            EmojiSupport::AddSupport(result->playerNameText);

            if (!cellBackgrounds.count(result)) {
                avatars[result] = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), BundleLoader::bundle->defaultAvatar, UnityEngine::Vector2(-30, 0), UnityEngine::Vector2(4, 4));
                avatars[result]->get_gameObject()->set_active(getModConfig().AvatarsActive.GetValue());

                auto scoreSelector = ::QuestUI::BeatSaberUI::CreateClickableImage(result->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6), [result]() {
                    auto openEvent = il2cpp_utils::MakeDelegate<System::Action *>(
                        classof(System::Action*),
                        static_cast<Il2CppObject *>(nullptr), setTheScoreAgain);
                    detailsTextWorkaround = cellScores[result];

                    scoreDetailsUI->modal->Show(true, true, openEvent);
                    scoreDetailsUI->setScore(cellScores[result]);
                });
                
                scoreSelector->set_material(UnityEngine::Object::Instantiate(BundleLoader::bundle->scoreUnderlineMaterial));
                
                cellHighlights[result] = scoreSelector;

                auto backgroundImage = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6));
                backgroundImage->set_material(BundleLoader::bundle->scoreBackgroundMaterial);
                backgroundImage->get_transform()->SetAsFirstSibling();
                cellBackgrounds[result] = backgroundImage;  

                // auto tagsList = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(result->get_transform());
                // clanGroups[result] = tagsList;         
            }
        }
        } else {
            if (result->scoreText->get_fontSize() == 2) {
                EmojiSupport::RemoveSupport(result->playerNameText);
                result->playerNameText->set_enableAutoSizing(true);
                resize(result->playerNameText, -24, 0);
                move(result->rankText, 6.2, 0.1);
                result->rankText->set_alignment(TMPro::TextAlignmentOptions::Left);
                move(result->playerNameText, 0.5, 0);
                move(result->fullComboText, -0.2, 0);
                move(result->scoreText, -4, 0);
                result->playerNameText->set_fontSize(4);
                result->fullComboText->set_fontSize(4);
                result->scoreText->set_fontSize(4);
            }
        }
        auto player = scoreVector[row].player;
        if (!isLocal && showBeatLeader) {
            if (cellBackgrounds.count(result)) {
            
            cellBackgrounds[result]->get_gameObject()->set_active(true);
            result->playerNameText->GetComponent<UnityEngine::RectTransform*>()->set_anchoredPosition({
                getModConfig().AvatarsActive.GetValue() ? 10.5f : 6.5f,
                result->playerNameText->GetComponent<UnityEngine::RectTransform*>()->get_anchoredPosition().y
            });
            avatars[result]->get_gameObject()->set_active(getModConfig().AvatarsActive.GetValue());
            result->scoreText->get_gameObject()->set_active(getModConfig().ScoresActive.GetValue());
            
            if (row == selectedScore) {
                cellBackgrounds[result]->set_color(ownScoreColor);
            } else {
                cellBackgrounds[result]->set_color(someoneElseScoreColor);
            }
            cellScores[result] = scoreVector[row];

            if(getModConfig().AvatarsActive.GetValue()){
                avatars[result]->set_sprite(BundleLoader::bundle->defaultAvatar);
                
                if (!PlayerController::IsIncognito(player)) {
                    Sprites::get_Icon(player.avatar, [result](UnityEngine::Sprite* sprite) {
                        if (sprite != NULL && avatars[result] != NULL && sprite->get_texture() != NULL) {
                            avatars[result]->set_sprite(sprite);
                        }
                    });
                }
            }

            // TODO
            // auto tagList = clanGroups[result];
            // for (int i = 0; i < tagList->get_transform()->get_childCount(); i++)
            //  UnityEngine::GameObject::Destroy(tagList->get_transform()->GetChild(i)->get_gameObject());
            // for (size_t i = 0; i < player.clans.size(); i++) {
            //     getLogger().info("%s", player.clans[i].tag.c_str());
            //     auto text = ::QuestUI::BeatSaberUI::CreateText(tagList->get_transform(), player.clans[i].tag, false);
            //     text->set_alignment(TMPro::TextAlignmentOptions::Center);
            //     auto background = text->get_gameObject()->AddComponent<HMUI::ImageView*>();
                
            //     background->set_material(BundleLoader::clanTagBackgroundMaterial);
            //     background->set_color(FormatUtils::hex2rgb(player.clans[i].color));
            // }

            auto scoreSelector = cellHighlights[result];
            scoreSelector->get_gameObject()->set_active(true);
            float hg = idleHighlight(player.role);
            scoreSelector->set_defaultColor(UnityEngine::Color(hg, 0.0, 0.0, 1.0));
            scoreSelector->set_highlightColor(underlineHoverColor);
            schemeForRole(player.role, false).Apply(scoreSelector->get_material());
            }
        } else {
            if (cellBackgrounds.count(result) && cellBackgrounds[result]) {
                cellBackgrounds[result]->get_gameObject()->set_active(false);
                avatars[result]->get_gameObject()->set_active(false);
                cellHighlights[result]->get_gameObject()->set_active(false);
            }
        }

        auto scoreSelector = cellHighlights[result];
        scoreSelector->get_gameObject()->set_active(true);
        float hg = idleHighlight(player.role);
        scoreSelector->set_defaultColor(UnityEngine::Color(hg, 0.0, 0.0, 1.0));
        scoreSelector->set_highlightColor(underlineHoverColor);
        schemeForRole(player.role, false).Apply(scoreSelector->get_material());

        return (TableCell *)result;
    }

    void updateStatus(ReplayUploadStatus status, string description, float progress, bool showRestart) {
        lastVotingStatusUrl = "";

        if (status != ReplayUploadStatus::inProgress) {
            updateVotingButton();
        }
        
        if (plvc && (plvc->isActivated || plvc->wasActivatedBefore)) {
            statusWasCached = false;
            uploadStatus->SetText(description);
            switch (status)
            {
                case ReplayUploadStatus::finished:
                    logoAnimation->SetAnimating(false);
                    Refresh();
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
                        uploadStatus->SetText("<color=#b103fcff>Posting replay: Finishing up...");
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

    void initContextsModal(UnityEngine::Transform* parent){
        auto container = QuestUI::BeatSaberUI::CreateModal(parent, {40, static_cast<float>((static_cast<int>(Context::Golf) + 2) * 10 + 5)}, nullptr, true);

        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Scores Context", {20, 19});

        for(int i = 0; i <= static_cast<int>(Context::Golf); i++)
        {
            QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), contextToDisplayString[static_cast<Context>(i)], {0.0f, static_cast<float>(21 - (i + 1) * 10)}, [i](){
                // Set the new value
                getModConfig().Context.SetValue(i);
                // Hide the modal
                contextsContainer->Hide(true, nullptr);
                // Clear the leaderboard
                clearTable();
                // Refresh the context button icon
                updateModifiersButton();
                // Fill the leaderboard
                refreshFromTheServer();
                // Refresh the player rank
                PlayerController::Refresh(0, [](auto player, auto str){
                    QuestUI::MainThreadScheduler::Schedule([]{
                        LeaderboardUI::updatePlayerRank();
                    });
                });
            });
        }

        contextsContainer = container;
    }

    void initSettingsModal(UnityEngine::Transform* parent){
        auto container = QuestUI::BeatSaberUI::CreateModal(parent, {40,50}, nullptr, true);
        
        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Leaderboard Settings", {16, 19});

        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Avatar", {12, 9});

        CreateToggle(container->get_transform(), getModConfig().AvatarsActive.GetValue(), {-3, 11}, [](bool value){
            getModConfig().AvatarsActive.SetValue(value);
            Refresh();
        });

        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Clans", {12, -1});

        CreateToggle(container->get_transform(), getModConfig().ClansActive.GetValue(), {-3, 1}, [](bool value){
            getModConfig().ClansActive.SetValue(value);
            Refresh();
        });

        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Score", {12, -11});

        CreateToggle(container->get_transform(), getModConfig().ScoresActive.GetValue(), {-3, -9}, [](bool value){
            getModConfig().ScoresActive.SetValue(value);
            Refresh();
        });

        QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Time", {12, -21});

        CreateToggle(container->get_transform(), getModConfig().TimesetActive.GetValue(), {-3, -19}, [](bool value){
            getModConfig().TimesetActive.SetValue(value);
            Refresh();
        });

        settingsContainer = container;
    }

    UnityEngine::UI::Toggle* CreateToggle(UnityEngine::Transform* parent, bool currentValue, UnityEngine::Vector2 anchoredPosition, std::function<void(bool)> onValueChange)
    {
        // Code adapted from: https://github.com/darknight1050/QuestUI/blob/master/src/BeatSaberUI.cpp#L826
        static SafePtrUnity<UnityEngine::UI::Toggle> toggleCopy;
        if(!toggleCopy){
            toggleCopy = Resources::FindObjectsOfTypeAll<UnityEngine::UI::Toggle*>().FirstOrDefault([](auto x) {return x->get_transform()->get_parent()->get_gameObject()->get_name() == "Fullscreen"; });
        }

        UnityEngine::UI::Toggle* newToggle = Object::Instantiate(toggleCopy.ptr(), parent, false);
        newToggle->set_interactable(true);
        newToggle->set_isOn(currentValue);
        newToggle->onValueChanged = UnityEngine::UI::Toggle::ToggleEvent::New_ctor();
        if(onValueChange)
            newToggle->onValueChanged->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction_1<bool>*>(onValueChange));
        RectTransform* rectTransform = newToggle->GetComponent<RectTransform*>();
        rectTransform->set_anchoredPosition(anchoredPosition);
        newToggle->get_gameObject()->set_active(true);
        return newToggle;
    }

    void setup() {
        if (hooksInstalled) return;

        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, LeaderboardActivate);
        INSTALL_HOOK(logger, LeaderboardCellSource);
        INSTALL_HOOK(logger, LeaderboardDeactivate);

        PlayerController::playerChanged.emplace_back([](std::optional<Player> const& updated) {
            QuestUI::MainThreadScheduler::Schedule([] {
                if (playerName != NULL) {
                    updatePlayerInfoLabel();
                }
            });
        });

        LeaderboardCore::Register::RegisterLeaderboard(&LeaderboardUI::leaderboard, modInfo);
        LeaderboardCore::Events::NotifyLeaderboardSet() += [](GlobalNamespace::IDifficultyBeatmap* newDifficultyBeatmap){
            difficultyBeatmap = newDifficultyBeatmap;
            if (plvc && plvc->isActivated) {
                Refresh();
            }
        };
    }

    void reset() {
        uploadStatus = NULL;
        plvc = NULL;
        originalplvc = NULL;
        scoreDetailsUI = NULL;
        votingUI = NULL;
        linkContainer = NULL;
        settingsContainer = NULL;
        contextsContainer = NULL;
        preferencesButton = NULL;
        parentScreen = NULL;
        cellScores.clear();
        avatars = {};
        cellHighlights = {};
        cellBackgrounds = {};
        showBeatLeaderButton = NULL;
        upPageButton = NULL;
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

    static const std::string bsml = R""""(
    <bg xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='https://raw.githubusercontent.com/RedBrumbler/Quest-BSML-Docs/gh-pages/schema.xsd'>
    <horizontal id="BLHeader" pref-width="91" pref-height="8" anchor-pos-y="41.5" horizontal-fit="PreferredSize" background="title-gradient">
        <text text="HIGHSCORES" font-size="6" align="Midline" italics="true" rich-text="true"/>
    </horizontal>
    <!-- Leaderboard scrollers -->
    <bg anchor-pos-x="-40" anchor-pos-y="-15">
        <vertical-icon-segments id="scopeSegmentedControl" contents='~leaderboardIcons' select-cell='OnIconSelected' anchor-pos-y="20" />
    </bg>

    <vertical horizontal-fit='PreferredSize' vertical-fit='PreferredSize' pref-width='76' pref-height='60' anchor-pos-x="7" anchor-pos-y="5.5">
        <leaderboard id='leaderboardTableView' cell-size='6.05'>
            <text id='loadingLabel' text="" align="Center"/>
        </leaderboard>
    </vertical>
    </bg>
    )"""";


    void LeaderboardViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation) {
            plvc = this;
            BSML::parse_and_construct(bsml, get_transform(), this);
            HMUI::ImageView* imageView = plvc->BLHeader->get_transform()->GetComponentInChildren<HMUI::ImageView*>();
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        }
        LeaderboardDidActivate();
        Refresh();
    }

    void LeaderboardViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling)
    {
        LeaderboardDidDeactivate();
    }

    void LeaderboardViewController::PostParse(){
        Array<IconSegmentedControl::DataItem*>* array = ::Array<IconSegmentedControl::DataItem*>::New({
            IconSegmentedControl::DataItem::New_ctor(BundleLoader::bundle->globeIcon, "Global"),
            IconSegmentedControl::DataItem::New_ctor(BundleLoader::bundle->defaultAvatar, "Around You"),
            IconSegmentedControl::DataItem::New_ctor(BundleLoader::bundle->friendsSelectorIcon, "Friends"),
            IconSegmentedControl::DataItem::New_ctor(BundleLoader::bundle->locationIcon, "Country"),
        });
        scopeSegmentedControl->SetData(array);
    }

    void LeaderboardViewController::OnIconSelected(IconSegmentedControl* segmentedControl, int index){
        Refresh();
    }
}