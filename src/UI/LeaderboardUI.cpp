#include "include/UI/LeaderboardUI.hpp"

#include "include/Models/Replay.hpp"
#include "include/Models/Score.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Enhancers/MapEnhancer.hpp"

#include "include/UI/UIUtils.hpp"
#include "include/UI/ScoreDetailsUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
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
#include "HMUI/CurvedTextMeshPro.hpp"

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
    PlatformLeaderboardViewController* leaderboardViewController;

    TMPro::TextMeshProUGUI* uploadStatus = NULL;

    TMPro::TextMeshProUGUI* playerName = NULL;
    HMUI::ImageView* playerAvatar = NULL;

    TMPro::TextMeshProUGUI* globalRank = NULL;
    HMUI::ImageView* globalRankIcon = NULL;
    TMPro::TextMeshProUGUI* countryRankAndPp = NULL;
    HMUI::ImageView* countryRankIcon = NULL;
    
    UnityEngine::UI::Button* retryButton = NULL;
    QuestUI::ClickableImage* websiteLink = NULL;
    PlatformLeaderboardViewController* plvc = NULL;

    QuestUI::ClickableImage* upPageButton = NULL;
    QuestUI::ClickableImage* downPageButton = NULL;
    UnityEngine::GameObject* parentScreen = NULL;

    HMUI::ModalView* scoreDetails = NULL;
    TMPro::TextMeshProUGUI* scorePlayerName = NULL;

    int page = 1;
    static vector<Score> scoreVector = vector<Score>(10);

    map<LeaderboardTableCell*, HMUI::ImageView*> avatars;
    map<LeaderboardTableCell*, QuestUI::ClickableImage*> cellBackgrounds;
    map<string, int> imageRows;

    static UnityEngine::Color lowAccColor = UnityEngine::Color(0.93, 1, 0.62, 1);
    static UnityEngine::Color highAccColor = UnityEngine::Color(1, 0.39, 0.28, 1);

    
    static UnityEngine::Color highlight = UnityEngine::Color(0.0, 0.4, 1.0, 0.8);

    std::string rgb2hex(UnityEngine::Color color) { 
        std::stringstream ss; 
        ss << std::hex << ((int)(color.r * 255.0) << 16 | (int)(color.g * 255.0) << 8 | (int)(color.b * 255.0)); 
        return ss.str();
    }

    string getAccColorString(float acc) {
        auto lerpValue = pow(acc, 14.0f);
        auto color = UnityEngine::Color::Lerp(lowAccColor, highAccColor, lerpValue);
        return rgb2hex(color);
    }

    string formatAcc(float accuracy) {
        return "<color=#" + getAccColorString(accuracy) + ">" + to_string_wprecision(accuracy * 100, 2) + "%";
    }

    string generateLabel(Score score) {
        string ppLabel = "<color=#B856FF>" + (score.pp > 0 ? to_string_wprecision(score.pp, 2) + "pp" : "");
        string nameLabel = score.player.name;
        string fcLabel =  "<color=#FFFFFF>" + (string)(score.fullCombo ? "FC" : "") + (score.modifiers.length() > 0 && score.fullCombo ? ", " : "") + score.modifiers;
        return truncate(nameLabel, 35) + "<pos=45%>" + ppLabel + "   " + formatAcc(score.accuracy) + " " + fcLabel; 
    }

    void updatePlayerInfoLabel() {
        Player* player = PlayerController::currentPlayer;
        if (player != NULL) {
            if (player->rank > 0) {

                globalRank->SetText("#" + to_string(player->rank));
                countryRankAndPp->SetText("#" + to_string(player->countryRank) + "       <color=#B856FF>" + to_string_wprecision(player->pp, 2) + "pp");
                playerName->SetText(player->name);

                Sprites::get_Icon(player->avatar, [](UnityEngine::Sprite* sprite) {
                    playerAvatar->set_sprite(sprite);
                });
                
                if (plvc != NULL) {
                    auto countryControl = plvc->scopeSegmentedControl->dataItems.get(2);
                    countryControl->set_hintText("Country");
                    Sprites::GetCountryIcon(player->country, [countryControl](UnityEngine::Sprite* sprite) {
                        plvc->scopeSegmentedControl->dataItems.get(2)->set_icon(sprite);
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
            playerAvatar->set_sprite(plvc->aroundPlayerLeaderboardIcon);
            countryRankIcon->set_sprite(plvc->friendsLeaderboardIcon);
            playerName->SetText("");
        }
    }

    MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
        LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        if (firstActivation) {
            HMUI::ImageView* imageView = self->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        }

        if (parentScreen != NULL) {
            parentScreen->SetActive(true);
        }
    }

    MAKE_HOOK_MATCH(LeaderboardDeactivate, &PlatformLeaderboardViewController::DidDeactivate, void, PlatformLeaderboardViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        LeaderboardDeactivate(self, removedFromHierarchy, screenSystemDisabling);

        if (parentScreen != NULL) {
            parentScreen->SetActive(false);
        }
    }

    //Work in progress to get stuff above leaderboard
    UnityEngine::GameObject* CreateCustomScreen(HMUI::ViewController* rootView, UnityEngine::Vector2 screenSize, UnityEngine::Vector3 position, float curvatureRadius) {
        auto gameObject = QuestUI::BeatSaberUI::CreateCanvas();
        auto screen = gameObject->AddComponent<HMUI::Screen*>();
        screen->rootViewController = rootView;
        auto curvedCanvasSettings = gameObject->AddComponent<HMUI::CurvedCanvasSettings*>();
        curvedCanvasSettings->SetRadius(curvatureRadius);

        auto transform = gameObject->get_transform();
        UnityEngine::GameObject* screenSystem = UnityEngine::GameObject::Find("ScreenContainer");
        if(screenSystem) {
            transform->set_position(screenSystem->get_transform()->get_position());
            screen->get_gameObject()->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta(screenSize);
        }
        return gameObject;
    }

    void refreshFromTheServer() {
        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(plvc->difficultyBeatmap->get_level());
        string hash = regex_replace((string)levelData->get_levelID(), basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(plvc->difficultyBeatmap->get_difficulty().value);
        string mode = (string)plvc->difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode;

        url += "/modifiers";

        switch (PlatformLeaderboardViewController::_get__scoresScope())
        {
        case PlatformLeaderboardsModel::ScoresScope::AroundPlayer:
            url += "/global/around";
            break;
        case PlatformLeaderboardsModel::ScoresScope::Friends:
            url += "/country/page";
            break;
        
        default:
            url += "/global/page";
            break;
        }

        url += "?page=" + to_string(page) + "&player=" + PlayerController::currentPlayer->id;

        WebUtils::GetJSONAsync(url, [](long status, bool error, rapidjson::Document& result){
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

            auto metadata = result["metadata"].GetObject();
            int perPage = metadata["itemsPerPage"].GetInt();
            int pageNum = metadata["page"].GetInt();
            int total = metadata["total"].GetInt();

            int selectedScore = 10;
            for (int index = 0; index < 10; ++index)
            {
                if (index < (int)scores.Size())
                {
                    auto score = scores[index].GetObject();
                    
                    Score currentScore = Score(score);
                    scoreVector[index] = currentScore;
                    
                    if (currentScore.playerId.compare(PlayerController::currentPlayer->id) == 0) {
                        selectedScore = index;
                    }

                    LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                        currentScore.modifiedScore, 
                        generateLabel(currentScore), 
                        currentScore.rank, 
                        false);
                    plvc->scores->Add(scoreData);
                }
            }
                
            plvc->leaderboardTableView->scores = plvc->scores;
            plvc->leaderboardTableView->specialScorePos = selectedScore;
            QuestUI::MainThreadScheduler::Schedule([pageNum, perPage, total] {
                upPageButton->get_gameObject()->SetActive(pageNum != 1);
                downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);

                plvc->loadingControl->Hide();
                plvc->hasScoresData = true;
                plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
            });
        });

        plvc->loadingControl->ShowText("Loading", true);
    }

    MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool showLoadingIndicator, bool clear) {
        leaderboardViewController = self;

        self->scores->Clear();
        self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
        page = 1;

        if (PlayerController::currentPlayer == NULL) {
            self->loadingControl->ShowText("Please sign up or log in mod settings!", true);
            return;
        }
        
        if (uploadStatus == NULL) {
            plvc = self;

            scoreDetails = ::QuestUI::BeatSaberUI::CreateModal(self->get_transform(), UnityEngine::Vector2(0, 0), [](HMUI::ModalView *modal) {}, true);
            scorePlayerName = ::QuestUI::BeatSaberUI::CreateText(scoreDetails->get_transform(), "", false, UnityEngine::Vector2(0, 0));

            parentScreen = CreateCustomScreen(self, UnityEngine::Vector2(400, 120), self->screen->get_transform()->get_position(), 140);

            playerAvatar = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->aroundPlayerLeaderboardIcon, UnityEngine::Vector2(180, 50), UnityEngine::Vector2(12, 12));
            globalRankIcon = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->globalLeaderboardIcon, UnityEngine::Vector2(130, 40), UnityEngine::Vector2(4, 4));
            countryRankIcon = ::QuestUI::BeatSaberUI::CreateImage(parentScreen->get_transform(), plvc->friendsLeaderboardIcon, UnityEngine::Vector2(140, 40), UnityEngine::Vector2(4, 4));
            playerName = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(140, 50));
            playerName->set_fontSize(6);
            globalRank = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(135, 40));
            countryRankAndPp = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(145, 40));

            if (PlayerController::currentPlayer != NULL) {
                updatePlayerInfoLabel();
            }

            if (websiteLink) UnityEngine::GameObject::Destroy(websiteLink);
            websiteLink = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(100, 50), UnityEngine::Vector2(12, 12), []() {
                string url = WebUtils::WEB_URL;
                if (PlayerController::currentPlayer != NULL) {
                    url += "u/" + PlayerController::currentPlayer->id;
                }
                UnityEngine::Application::OpenURL(url);
            });

            if (retryButton) UnityEngine::GameObject::Destroy(retryButton);
            retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(130, 44), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                retryCallback();
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

            if(uploadStatus) UnityEngine::GameObject::Destroy(uploadStatus);
            uploadStatus = ::QuestUI::BeatSaberUI::CreateText(parentScreen->get_transform(), "", false);
            move(uploadStatus, 140, 50);
            resize(uploadStatus, 10, 0);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);

            upPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), Sprites::get_UpIcon(), UnityEngine::Vector2(100, 20), UnityEngine::Vector2(8, 5.12), [](){
                page--;
                refreshFromTheServer();
            });
            downPageButton = ::QuestUI::BeatSaberUI::CreateClickableImage(parentScreen->get_transform(), Sprites::get_DownIcon(), UnityEngine::Vector2(100, -20), UnityEngine::Vector2(8, 5.12), [](){
                page++;
                refreshFromTheServer();
            });
        }

        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(self->difficultyBeatmap->get_level());
        if (!levelData->get_levelID().starts_with("custom_level")) {
            self->loadingControl->Hide();
            self->hasScoresData = false;
            self->loadingControl->ShowText("Leaderboards for this map are not supported!", false);
            self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
        } else {
            refreshFromTheServer();
        }
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, HMUI::TableCell*, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

        if (result->playerNameText->get_fontSize() > 3) {
            result->playerNameText->set_enableAutoSizing(false);
            result->playerNameText->set_richText(true);
            resize(result->playerNameText, 13, 0);
            move(result->playerNameText, -2, 0);
            move(result->fullComboText, 0.2, 0);
            move(result->scoreText, 5, 0);
            result->playerNameText->set_fontSize(3);
            result->fullComboText->set_fontSize(3);
            result->scoreText->set_fontSize(2);

            avatars[result] = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), plvc->aroundPlayerLeaderboardIcon, UnityEngine::Vector2(-32, 0), UnityEngine::Vector2(4, 4));
            cellBackgrounds[result] = ::QuestUI::BeatSaberUI::CreateClickableImage(result->get_transform(), Sprites::get_CellBG(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(100, 6), [row]() {
                scorePlayerName->SetText(scoreVector[row].player.name);
                scoreDetails->Show(true, true, nullptr);
                //levelSelectCoordinator->SetRightScreenViewController(levelStatsView, HMUI::ViewController::AnimationType::In);
            });
            cellBackgrounds[result]->set_highlightColor(highlight);
            // cellBackgrounds[result]->get_transform()->SetAsFirstSibling();
        }

        avatars[result]->set_sprite(plvc->aroundPlayerLeaderboardIcon);
        Sprites::get_Icon(scoreVector[row].player.avatar, [result](UnityEngine::Sprite* sprite) {
            if (sprite != NULL && avatars[result] != NULL && sprite->get_texture() != NULL) {
                avatars[result]->set_sprite(sprite);
            }
        });

        return (TableCell *)result;
    }

    void updateStatus(ReplayUploadStatus status, string description, float progress) {
        uploadStatus->SetText(description);
        switch (status)
        {
            case ReplayUploadStatus::finished:
                leaderboardViewController->Refresh(true, true);
                break;
            case ReplayUploadStatus::error:
                retryButton->get_gameObject()->SetActive(true);
                break;
            case ReplayUploadStatus::inProgress:
                if (progress >= 100)
                    uploadStatus->SetText("<color=#b103fcff>Posting replay: Finishing up...");
                break;
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");

        INSTALL_HOOK(logger, LeaderboardActivate);
        INSTALL_HOOK(logger, LeaderboardDeactivate);
        INSTALL_HOOK(logger, RefreshLeaderboard);
        INSTALL_HOOK(logger, LeaderboardCellSource);

        PlayerController::playerChanged.push_back([](Player* updated) {
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
        avatars = map<LeaderboardTableCell*, HMUI::ImageView*>();
    }    
}