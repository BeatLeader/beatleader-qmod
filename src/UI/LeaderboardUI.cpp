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

#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/ModConfig.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

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

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/PlatformLeaderboardsModel.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LocalLeaderboardViewController.hpp"
#include "GlobalNamespace/LoadingControl.hpp"
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

#include "main.hpp"

#include <regex>
#include <map>

using namespace GlobalNamespace;
using namespace HMUI;
using namespace QuestUI;
using namespace BeatLeader;
using UnityEngine::Resources;

namespace LeaderboardUI {
    function<void()> retryCallback;
    PlatformLeaderboardViewController* plvc = NULL;

    TMPro::TextMeshProUGUI* uploadStatus = NULL;

    TMPro::TextMeshProUGUI* playerName = NULL;
    BeatLeader::PlayerAvatar* playerAvatar = NULL;

    TMPro::TextMeshProUGUI* globalRank = NULL;
    HMUI::ImageView* globalRankIcon = NULL;
    TMPro::TextMeshProUGUI* countryRankAndPp = NULL;
    HMUI::ImageView* countryRankIcon = NULL;
    
    UnityEngine::UI::Button* retryButton = NULL;
    BeatLeader::LogoAnimation* logoAnimation = NULL;
    
    QuestUI::ClickableImage* upPageButton = NULL;
    QuestUI::ClickableImage* downPageButton = NULL;
    QuestUI::ClickableImage* modifiersButton = NULL;
    BeatLeader::VotingButton* votingButton = NULL;
    UnityEngine::GameObject* parentScreen = NULL;

    TMPro::TextMeshProUGUI* loginPrompt = NULL;
    UnityEngine::UI::Button* preferencesButton = NULL;

    BeatLeader::ScoreDetailsPopup* scoreDetailsUI = NULL;
    BeatLeader::RankVotingPopup* votingUI = NULL;
    BeatLeader::LinksContainerPopup* linkContainer = NULL;
    bool visible = false;

    int page = 1;
    int selectedScore = 11;
    bool modifiers = true;
    static vector<Score> scoreVector = vector<Score>(11);

    map<LeaderboardTableCell*, HMUI::ImageView*> avatars;
    map<LeaderboardTableCell*, HMUI::ImageView*> cellBackgrounds;
    map<LeaderboardTableCell*, QuestUI::ClickableImage*> cellHighlights;
    map<LeaderboardTableCell*, Score> cellScores;
    map<string, int> imageRows;

    static UnityEngine::Color underlineHoverColor = UnityEngine::Color(1.0, 0.0, 0.0, 1.0);

    static UnityEngine::Color ownScoreColor = UnityEngine::Color(0.7, 0.0, 0.7, 0.3);
    static UnityEngine::Color someoneElseScoreColor = UnityEngine::Color(0.07, 0.0, 0.14, 0.05);

    static string lastUrl = "";
    static string lastVotingStatusUrl = "";
    static string votingUrl = "";

    void updatePlayerInfoLabel() {
        auto const& player = PlayerController::currentPlayer;
        if (player != std::nullopt) {
            if (player->rank > 0) {

                globalRank->SetText("#" + to_string(player->rank));
                countryRankAndPp->SetText("#" + to_string(player->countryRank) + "        <color=#B856FF>" + to_string_wprecision(player->pp, 2) + "pp");
                playerName->set_alignment(TMPro::TextAlignmentOptions::Center);
                playerName->SetText(FormatUtils::FormatNameWithClans(PlayerController::currentPlayer.value(), 25));
                playerAvatar->SetPlayer(player->avatar, player->role);
                
                if (plvc != NULL) {
                    auto countryControl = plvc->scopeSegmentedControl->dataItems.get(3);
                    countryControl->set_hintText("Country");
                    Sprites::GetCountryIcon(player->country, [](UnityEngine::Sprite* sprite) {
                        plvc->scopeSegmentedControl->dataItems.get(3)->set_icon(sprite);
                        plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);

                        countryRankIcon->set_sprite(sprite);
                    });
                    plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);
                }

            } else {
                playerName->SetText(player->name + ", play something!");
            }
        } else {
            globalRank->SetText("#0");
            countryRankAndPp->SetText("#0");
            // playerAvatar->set_sprite(plvc->aroundPlayerLeaderboardIcon);
            countryRankIcon->set_sprite(plvc->friendsLeaderboardIcon);
            playerName->SetText("");
        }
    }

    MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
        LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        if (firstActivation) {
            HMUI::ImageView* imageView = self->get_transform()->Find("HeaderPanel")->GetComponentInChildren<HMUI::ImageView*>();
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        }

        if (parentScreen != NULL) {
            visible = true;
            parentScreen->SetActive(true);
        }
    }

    MAKE_HOOK_MATCH(LeaderboardDeactivate, &PlatformLeaderboardViewController::DidDeactivate, void, PlatformLeaderboardViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        LeaderboardDeactivate(self, removedFromHierarchy, screenSystemDisabling);

        if (parentScreen != NULL) {
            visible = false;
            parentScreen->SetActive(false);
        }
    }

    void updateVotingButton(string votingStatusUrl) {
        votingButton->SetState(0);
        votingUI->modal->Hide(true, nullptr);

        lastVotingStatusUrl = votingStatusUrl;
        WebUtils::GetAsync(votingStatusUrl, [votingStatusUrl](long status, string response) {
            if (votingStatusUrl == lastVotingStatusUrl && status == 200) {
                votingButton->SetState(stoi(response));
            }
        }, [](float progress){});
    }

    void openSettings() {
        auto currentFlowCoordinator = BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
        
        auto modSettingsFlowCoordinator = GetModSettingsFlowCoordinator();
        if (modSettingsFlowCoordinator == NULL) {
            modSettingsFlowCoordinator = BeatSaberUI::CreateFlowCoordinator(reinterpret_cast<System::Type*>(il2cpp_utils::GetSystemType(il2cpp_utils::GetClassFromName("QuestUI", "ModSettingsFlowCoordinator"))));
        }
        
        currentFlowCoordinator->PresentFlowCoordinator(modSettingsFlowCoordinator, nullptr, ViewController::AnimationDirection::Horizontal, true, false);

        QuestUI::MainThreadScheduler::Schedule([modSettingsFlowCoordinator] {
            auto buttons = modSettingsFlowCoordinator->get_topViewController()->GetComponentsInChildren<UnityEngine::UI::Button*>();
            for (size_t i = 0; i < buttons.size(); i++)
            {
                auto textMesh = buttons[i]->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
                if (textMesh->get_text() == "bl" || textMesh->get_text() == "BeatLeader") {
                    buttons[i]->get_onClick()->Invoke();
                }
            }
        });
    }

    void refreshFromTheServer() {
        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(plvc->difficultyBeatmap->get_level());
        string hash = regex_replace((string)levelData->get_levelID(), basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(plvc->difficultyBeatmap->get_difficulty().value);
        string mode = (string)plvc->difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode;

        if (modifiers) {
            url += "/modifiers";
        } else {
            url += "/standard";
        }

        switch (PlatformLeaderboardViewController::_get__scoresScope())
        {
        case PlatformLeaderboardsModel::ScoresScope::AroundPlayer:
            url += "/global/around";
            break;
        case PlatformLeaderboardsModel::ScoresScope::Friends:
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

        WebUtils::GetJSONAsync(url, [url](long status, bool error, rapidjson::Document const& result){
            if (url != lastUrl) return;

            if (status != 200 || error || !result.HasMember("data")) {
                return;
            } 

            auto scores = result["data"].GetArray();
            plvc->scores->Clear();
            if ((int)scores.Size() == 0) {
                QuestUI::MainThreadScheduler::Schedule([status] {
                    plvc->loadingControl->Hide();
                    plvc->hasScoresData = false;
                    plvc->loadingControl->ShowText("No scores were found!", true);
                    
                    plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
                });
                return;
            }

            auto const& metadata = result["metadata"].GetObject();
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
                    plvc->scores->Add(scoreData);
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
                    plvc->scores->Add(scoreData);
                    scoreVector[10] = currentScore;
                    selectedScore = 10;
                } else {
                    for (size_t i = 10; i > 0; i--)
                    {
                        scoreVector[i] = scoreVector[i - 1];
                    }
                    plvc->scores->Insert(0, scoreData);
                    scoreVector[0] = currentScore;
                    selectedScore = 0;
                }
                if (plvc->scores->get_Count() > 10) {
                    plvc->leaderboardTableView->rowHeight = 5.5;
                }
            }
                
            plvc->leaderboardTableView->scores = plvc->scores;
            plvc->leaderboardTableView->specialScorePos = 12;
            QuestUI::MainThreadScheduler::Schedule([pageNum, perPage, total] {
                upPageButton->get_gameObject()->SetActive(pageNum != 1);
                downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);

                plvc->loadingControl->Hide();
                plvc->hasScoresData = true;
                plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
            });
        });
        
        string votingStatusUrl = WebUtils::API_URL + "votestatus/" + hash + "/" + difficulty + "/" + mode;
        votingUrl = WebUtils::API_URL + "vote/" + hash + "/" + difficulty + "/" + mode;
        if (lastVotingStatusUrl != votingStatusUrl) {
            updateVotingButton(votingStatusUrl);
        }

        plvc->loadingControl->ShowText("Loading", true);
    }

    static UnityEngine::Color selectedColor = UnityEngine::Color(0.0, 0.4, 1.0, 1.0);
    static UnityEngine::Color fadedColor = UnityEngine::Color(0.8, 0.8, 0.8, 0.2);
    static UnityEngine::Color fadedHoverColor = UnityEngine::Color(0.5, 0.5, 0.5, 0.2);

    void updateModifiersButton() {
        modifiersButton->set_defaultColor(modifiers ? selectedColor : fadedColor);
        modifiersButton->set_highlightColor(modifiers ? selectedColor : fadedHoverColor);

        if (modifiers) {
            ::QuestUI::BeatSaberUI::AddHoverHint(modifiersButton, "Show leaderboard without positive modifiers");
        } else {
            ::QuestUI::BeatSaberUI::AddHoverHint(modifiersButton, "Show leaderboard with positive modifiers");
        }
    }

    void voteCallback(bool voted, bool rankable, float stars, int type) {
        if (voted) {
            votingButton->SetState(0);
            string rankableString = "?rankability=" + (rankable ? (string)"1.0" : (string)"0.0");
            string starsString = stars > 0 ? "&stars=" + to_string_wprecision(stars, 2) : "";
            string typeString = type > 0 ? "&type=" + to_string(type) : "";
            string currentVotingUrl = votingUrl;
            WebUtils::PostJSONAsync(votingUrl + rankableString + starsString + typeString, "", [currentVotingUrl](long status, string response) {
                if (votingUrl != currentVotingUrl) return;

                if (status == 200) {
                    votingButton->SetState(stoi(response));
                } else {
                    votingButton->SetState(1);
                }
            });
        }

        votingUI->modal->Hide(true, nullptr);
    }

    static bool isLocal = false;

    void clearTable() {
        selectedScore = 11;
        plvc->scores->Clear();
        plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
    }

    MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool showLoadingIndicator, bool clear) {
        plvc = self;
        clearTable();
        page = 1;
        isLocal = false;

        if (PlayerController::currentPlayer == std::nullopt) {
            self->loadingControl->Hide();
            
            if (preferencesButton == NULL) {
                loginPrompt = ::QuestUI::BeatSaberUI::CreateText(plvc->get_transform(), "Please sign up or log in to post scores!", false, UnityEngine::Vector2(4, 10));
                preferencesButton = ::QuestUI::BeatSaberUI::CreateUIButton(plvc->get_transform(), "Open settings", UnityEngine::Vector2(0, 0), [](){
                    openSettings();
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
            ArrayW<::HMUI::IconSegmentedControl::DataItem*> dataItems = ArrayW<::HMUI::IconSegmentedControl::DataItem*>(4);
            ArrayW<PlatformLeaderboardsModel::ScoresScope> scoreScopes = ArrayW<PlatformLeaderboardsModel::ScoresScope>(4);
            for (int index = 0; index < 3; ++index)
            {
                dataItems[index] = self->scopeSegmentedControl->dataItems.get(index);
                scoreScopes[index] = self->scoreScopes.get(index);
            }
            dataItems[3] = HMUI::IconSegmentedControl::DataItem::New_ctor(self->friendsLeaderboardIcon, "Country");
            scoreScopes[3] = PlatformLeaderboardsModel::ScoresScope(3);

            self->scopeSegmentedControl->SetData(dataItems);
            self->scoreScopes = scoreScopes;

            parentScreen = CreateCustomScreen(self, UnityEngine::Vector2(480, 160), self->screen->get_transform()->get_position(), 140);
            visible = true;

            BeatLeader::initScoreDetailsPopup(&scoreDetailsUI, self->get_transform());
            BeatLeader::initLinksContainerPopup(&linkContainer, self->get_transform());
            BeatLeader::initVotingPopup(&votingUI, self->get_transform(), voteCallback);

            auto playerAvatarImage = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->aroundPlayerLeaderboardIcon, UnityEngine::Vector2(180, 51), UnityEngine::Vector2(20, 20));
            playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
            playerAvatar->Init(playerAvatarImage);

            globalRankIcon = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->globalLeaderboardIcon, UnityEngine::Vector2(120, 45), UnityEngine::Vector2(4, 4));
            countryRankIcon = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->friendsLeaderboardIcon, UnityEngine::Vector2(135, 45), UnityEngine::Vector2(4, 4));
            playerName = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(140, 53), UnityEngine::Vector2(60, 10));
            playerName->set_fontSize(6);

            EmojiSupport::AddSupport(playerName);
            
            globalRank = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(153, 42.5));
            countryRankAndPp = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(168, 42.5));

            if (PlayerController::currentPlayer != std::nullopt) {
                updatePlayerInfoLabel();
            }

            auto websiteLink = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, UnityEngine::Vector2(100, 50), UnityEngine::Vector2(12, 12), []() {
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
            retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(105, 63), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                retryCallback();
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

            if(uploadStatus) UnityEngine::GameObject::Destroy(uploadStatus);
            uploadStatus = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false);
            move(uploadStatus, 150, 60);
            resize(uploadStatus, 10, 0);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);

            upPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), Sprites::get_UpIcon(), UnityEngine::Vector2(100, 17), UnityEngine::Vector2(8, 5.12), [](){
                page--;
                clearTable();
                refreshFromTheServer();
            });
            downPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), Sprites::get_DownIcon(), UnityEngine::Vector2(100, -17), UnityEngine::Vector2(8, 5.12), [](){
                page++;
                clearTable();
                refreshFromTheServer();
            });

            modifiersButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->modifiersIcon, UnityEngine::Vector2(100, 28), UnityEngine::Vector2(4, 4), [](){
                modifiers = !modifiers;
                getModConfig().Modifiers.SetValue(modifiers);
                clearTable();
                updateModifiersButton();
                refreshFromTheServer();
            });
            modifiers = getModConfig().Modifiers.GetValue();
            ::QuestUI::BeatSaberUI::AddHoverHint(modifiersButton, "Show leaderboard without positive modifiers");
            updateModifiersButton();

            auto votingButtonImage = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->modifiersIcon, UnityEngine::Vector2(100, 22), UnityEngine::Vector2(4, 4), []() {
                if (votingButton->state != 2) return;
                
                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            votingButton = websiteLink->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            votingButton->Init(votingButtonImage);
        }

        upPageButton->get_gameObject()->SetActive(false);
        downPageButton->get_gameObject()->SetActive(false);

        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(self->difficultyBeatmap->get_level());
        if (!levelData->get_levelID().starts_with("custom_level")) {
            votingButton->SetState(-1);
            self->loadingControl->Hide();
            self->hasScoresData = false;
            self->loadingControl->ShowText("Leaderboards for this map are not supported!", false);
            self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
        } else {
            refreshFromTheServer();
        }
    }

    Score detailsTextWorkaround;

    void setTheScoreAgain() {
        scoreDetailsUI->setScore(detailsTextWorkaround);
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, HMUI::TableCell*, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

        if (!isLocal && result->playerNameText->get_fontSize() > 3) {
            result->playerNameText->set_enableAutoSizing(false);
            result->playerNameText->set_richText(true);
            EmojiSupport::AddSupport(result->playerNameText);
            resize(result->playerNameText, 24, 0);
            move(result->rankText, -6.2, -0.1);
            result->rankText->set_alignment(TMPro::TextAlignmentOptions::Right);

            move(result->playerNameText, -0.5, 0);
            move(result->fullComboText, 0.2, 0);
            move(result->scoreText, 4, 0);
            result->playerNameText->set_fontSize(3);
            result->fullComboText->set_fontSize(3);
            result->scoreText->set_fontSize(2);

            avatars[result] = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), plvc->aroundPlayerLeaderboardIcon, UnityEngine::Vector2(-30, 0), UnityEngine::Vector2(4, 4));

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

        if (!isLocal) {
            auto player = scoreVector[row].player;
            cellBackgrounds[result]->get_gameObject()->set_active(true);
            avatars[result]->get_gameObject()->set_active(true);
            
            if (row == selectedScore) {
                cellBackgrounds[result]->set_color(ownScoreColor);
            } else {
                cellBackgrounds[result]->set_color(someoneElseScoreColor);
            }
            cellScores[result] = scoreVector[row];
            avatars[result]->set_sprite(plvc->aroundPlayerLeaderboardIcon);
            
            Sprites::get_Icon(player.avatar, [result](UnityEngine::Sprite* sprite) {
                if (sprite != NULL && avatars[result] != NULL && sprite->get_texture() != NULL) {
                    avatars[result]->set_sprite(sprite);
                }
            });

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
        } else {
            if (cellBackgrounds.count(result)) {
                cellBackgrounds[result]->get_gameObject()->set_active(false);
                avatars[result]->get_gameObject()->set_active(false);
                cellHighlights[result]->get_gameObject()->set_active(false);
            }
        }

        return (TableCell *)result;
    }

    void updateStatus(ReplayUploadStatus status, string description, float progress, bool showRestart) {
        lastVotingStatusUrl = "";
        if (visible) {
            uploadStatus->SetText(description);
            switch (status)
            {
                case ReplayUploadStatus::finished:
                    logoAnimation->SetAnimating(false);
                    updateVotingButton(lastVotingStatusUrl);
                    plvc->HandleDidPressRefreshButton();
                    break;
                case ReplayUploadStatus::error:
                    logoAnimation->SetAnimating(false);
                    if (showRestart) {
                        retryButton->get_gameObject()->SetActive(true);
                    }
                    break;
                case ReplayUploadStatus::inProgress:
                    logoAnimation->SetAnimating(true);
                    if (progress >= 100)
                        uploadStatus->SetText("<color=#b103fcff>Posting replay: Finishing up...");
                    break;
            }
        }
       
    }

    MAKE_HOOK_MATCH(LocalLeaderboardDidActivate, &LocalLeaderboardViewController::DidActivate, void, LocalLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        isLocal = true;

        LocalLeaderboardDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, LeaderboardActivate);
        INSTALL_HOOK(logger, LeaderboardDeactivate);
        INSTALL_HOOK(logger, RefreshLeaderboard);
        INSTALL_HOOK(logger, LeaderboardCellSource);
        INSTALL_HOOK(logger, LocalLeaderboardDidActivate);

        PlayerController::playerChanged.emplace_back([](std::optional<Player> const& updated) {
            QuestUI::MainThreadScheduler::Schedule([] {
                if (playerName != NULL) {
                    updatePlayerInfoLabel();
                }
            });
        });
    }

    void reset() {
        uploadStatus = NULL;
        plvc = NULL;
        scoreDetailsUI = NULL;
        votingUI = NULL;
        linkContainer = NULL;
        cellScores.clear();
        avatars = {};
        cellHighlights = {};
        cellBackgrounds = {};
    }    
}