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
#include "include/UI/LogoAnimation.hpp"
#include "include/UI/PlayerAvatar.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ClickableImage.hpp"
#include "include/UI/RoleColorScheme.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LeaderboardUI.hpp"
#include "include/UI/ModifiersUI.hpp"

#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/ModConfig.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"
#include "UI/ClickableImage.hpp"

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
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/Events/UnityAction.hpp"

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
#include "GlobalNamespace/MenuShockwave.hpp"

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

using HapticPresetSO = Libraries::HM::HMLib::VR::HapticPresetSO;
static SafePtr<HapticPresetSO> hapticFeedbackPresetSO;

QuestUI::ClickableImage* CreateClickableImage(UnityEngine::Transform* parent, UnityEngine::Sprite* sprite, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick)
    {
        auto go = UnityEngine::GameObject::New_ctor(il2cpp_utils::createcsstr("QuestUIClickableImage"));

        auto image = go->AddComponent<QuestUI::ClickableImage*>();
        auto mat_UINoGlows = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>();
        UnityEngine::Material* mat_UINoGlow = NULL;
        for (int i = 0; i < mat_UINoGlows->Length(); i++) {
            if (to_utf8(csstrtostr(mat_UINoGlows->get(i)->get_name())) == "UINoGlow") {
                mat_UINoGlow = mat_UINoGlows->get(i);
                break;
            }
        }

        image->set_material(mat_UINoGlow);

        go->get_transform()->SetParent(parent, false);
        image->get_rectTransform()->set_sizeDelta(sizeDelta);
        image->get_rectTransform()->set_anchoredPosition(anchoredPosition);
        image->set_sprite(sprite);

        go->AddComponent<UnityEngine::UI::LayoutElement*>();

        if (onClick)
            image->get_onPointerClickEvent() += [onClick](auto _){ onClick(); };
        
        try
        {
            auto menuShockWave = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuShockwave*>()->get(0);
            auto buttonClickedSignal = menuShockWave->dyn__buttonClickEvents()->get(menuShockWave->dyn__buttonClickEvents()->Length() - 1);
            image->buttonClickedSignal = buttonClickedSignal;
        }
        catch(const std::exception& e)
        {
            getLogger().error("%s", e.what());
        }

        if (!hapticFeedbackPresetSO)
        {
            hapticFeedbackPresetSO.emplace(UnityEngine::ScriptableObject::CreateInstance<HapticPresetSO*>());
            hapticFeedbackPresetSO->duration = 0.01f;
            hapticFeedbackPresetSO->strength = 0.75f;
            hapticFeedbackPresetSO->frequency = 0.5f;
        }

        auto hapticFeedbackController = UnityEngine::Object::FindObjectOfType<GlobalNamespace::HapticFeedbackController*>();
        image->hapticFeedbackController = hapticFeedbackController;
        image->hapticFeedbackPresetSO = (HapticPresetSO*)hapticFeedbackPresetSO;

        return image;
    }

    QuestUI::ClickableImage* CreateClickableImage(UnityEngine::Transform* parent, UnityEngine::Sprite* sprite, std::function<void()> onClick)
    {
        return CreateClickableImage(parent, sprite, {0, 0}, {0, 0}, onClick);
    }

namespace LeaderboardUI {
    function<void()> retryCallback;
    PlatformLeaderboardViewController* plvc = NULL;

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
    QuestUI::ClickableImage* modifiersButton = NULL;
    HMUI::HoverHint* modifiersButtonHover;
    BeatLeader::VotingButton* votingButton = NULL;
    UnityEngine::GameObject* parentScreen = NULL;

    BeatLeader::ScoreDetailsPopup* scoreDetailsUI = NULL;
    BeatLeader::RankVotingPopup* votingUI = NULL;

    int page = 1;
    bool showRetryButton = false;
    int selectedScore = 11;
    bool modifiers = true;
    static vector<Score> scoreVector = vector<Score>(10);

    map<LeaderboardTableCell*, HMUI::ImageView*> avatars;
    map<LeaderboardTableCell*, HMUI::ImageView*> cellBackgrounds;
    map<LeaderboardTableCell*, QuestUI::ClickableImage*> cellHighlights;
    map<LeaderboardTableCell*, Score> cellScores;
    map<string, int> imageRows;

    static UnityEngine::Color underlineHoverColor = UnityEngine::Color(1.0, 0.0, 0.0, 1.0);

    static UnityEngine::Color ownScoreColor = UnityEngine::Color(0.7, 0.0, 0.7, 0.3);
    static UnityEngine::Color someoneElseScoreColor = UnityEngine::Color(0.07, 0.0, 0.14, 0.05);

    static bool bundleLoaded = false;
    static string lastUrl = "";
    static string lastVotingStatusUrl = "";
    static string votingUrl = "";

    static vector<UnityEngine::Transform*> ssElements;
    bool ssInstalled = true;
    bool ssWasOpened = false;
    bool showBeatLeader = false;
    bool restoredFromPreferences = false;

    UnityEngine::UI::Button* sspageUpButton;
    UnityEngine::UI::Button::ButtonClickedEvent* ssUpAction;
    UnityEngine::UI::Button::ButtonClickedEvent* blUpAction;
    UnityEngine::UI::Button* sspageDownButton;
    UnityEngine::UI::Button::ButtonClickedEvent* ssDownAction;
    UnityEngine::UI::Button::ButtonClickedEvent* blDownAction;

    string generateLabel(Score const& score) {
        // TODO: Use fmt
        string const& nameLabel = score.player.name;

        string fcLabel = "<color=#FFFFFF>" + (string)(score.fullCombo ? "FC" : "") + (score.modifiers.length() > 0 && score.fullCombo ? "," : "") + score.modifiers;
        return FormatUtils::FormatNameWithClans(score.player) + "<pos=45%>" + FormatUtils::FormatPP(score.pp) + "   " + FormatUtils::formatAcc(score.accuracy) + " " + fcLabel; 
    }

    void updatePlayerInfoLabel() {
        auto const& player = PlayerController::currentPlayer;
        if (player != std::nullopt) {
            if (player->rank > 0) {

                globalRank->SetText(il2cpp_utils::createcsstr("#" + to_string(player->rank)));
                countryRankAndPp->SetText(il2cpp_utils::createcsstr("#" + to_string(player->countryRank) + "        <color=#B856FF>" + to_string_wprecision(player->pp, 2) + "pp"));
                playerName->set_alignment(TMPro::TextAlignmentOptions::Center);
                playerName->SetText(il2cpp_utils::createcsstr(FormatUtils::FormatNameWithClans(PlayerController::currentPlayer.value())));
                playerAvatar->SetPlayer(il2cpp_utils::createcsstr(player->avatar), il2cpp_utils::createcsstr(player->role));
                
                if (plvc != NULL) {
                    auto countryControl = plvc->scopeSegmentedControl->dataItems->get(3);
                    countryControl->set_hintText(il2cpp_utils::createcsstr("Country"));
                    Sprites::GetCountryIcon(player->country, [countryControl](UnityEngine::Sprite* sprite) {
                        plvc->scopeSegmentedControl->dataItems->get(3)->set_icon(sprite);
                        plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);

                        countryRankIcon->set_sprite(sprite);
                    });
                    plvc->scopeSegmentedControl->SetData(plvc->scopeSegmentedControl->dataItems);
                }

            } else {
                playerName->SetText(il2cpp_utils::createcsstr(player->name + ", play something!"));
            }
        } else {
            globalRank->SetText(il2cpp_utils::createcsstr("#0"));
            countryRankAndPp->SetText(il2cpp_utils::createcsstr("#0"));
            // playerAvatar->set_sprite(plvc->aroundPlayerLeaderboardIcon);
            countryRankIcon->set_sprite(plvc->friendsLeaderboardIcon);
            playerName->SetText(il2cpp_utils::createcsstr(""));
        }
    }

    MAKE_HOOK_MATCH(LeaderboardActivate, &PlatformLeaderboardViewController::DidActivate, void, PlatformLeaderboardViewController* self, bool firstActivation, bool addedToHeirarchy, bool screenSystemEnabling) {
        LeaderboardActivate(self, firstActivation, addedToHeirarchy, screenSystemEnabling);
        if (firstActivation) {
            HMUI::ImageView* imageView = self->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
            imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
            imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
            imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
        }

        plvc = self;

        if (parentScreen != NULL) {
            parentScreen->SetActive(showBeatLeader);
        }
    }

    MAKE_HOOK_MATCH(LeaderboardDeactivate, &PlatformLeaderboardViewController::DidDeactivate, void, PlatformLeaderboardViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
        LeaderboardDeactivate(self, removedFromHierarchy, screenSystemDisabling);

        bundleLoaded = false;
        
        if (parentScreen != NULL) {
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

    UnityEngine::GameObject* CreateCustomScreen(HMUI::ViewController* rootView, UnityEngine::Vector2 screenSize, UnityEngine::Vector3 position, float curvatureRadius) {
        auto gameObject = QuestUI::BeatSaberUI::CreateCanvas();
        auto screen = gameObject->AddComponent<HMUI::Screen*>();
        screen->rootViewController = rootView;
        auto curvedCanvasSettings = gameObject->AddComponent<HMUI::CurvedCanvasSettings*>();
        curvedCanvasSettings->SetRadius(curvatureRadius);

        auto transform = gameObject->get_transform();
        UnityEngine::GameObject* screenSystem = UnityEngine::GameObject::Find(il2cpp_utils::createcsstr("ScreenContainer"));
        if(screenSystem) {
            transform->set_position(screenSystem->get_transform()->get_position());
            screen->get_gameObject()->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta(screenSize);
        }
        return gameObject;
    }

    void refreshFromTheServer() {
        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(plvc->difficultyBeatmap->get_level());
        if (levelData == NULL) return;
        string hash = regex_replace(to_utf8(csstrtostr(levelData->get_levelID())), basic_regex("custom_level_"), "");
        string difficulty = MapEnhancer::DiffName(plvc->difficultyBeatmap->get_difficulty().value);
        string mode = to_utf8(csstrtostr(plvc->difficultyBeatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName));
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode;

        if (modifiers) {
            url += "/modifiers";
        } else {
            url += "/standard";
        }

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

        WebUtils::GetJSONAsync(url, [url](long status, bool error, rapidjson::Document const& result){
            if (url != lastUrl) return; 
            if (!showBeatLeader) return;

            auto scores = result["data"].GetArray();
            
            if ((int)scores.Size() == 0) {
                QuestUI::MainThreadScheduler::Schedule([status] {
                    plvc->loadingControl->Hide();
                    plvc->hasScoresData = false;
                    if (plvc->leaderboardTableView->scores != NULL) {
                        plvc->leaderboardTableView->scores->Clear();
                    }
                    plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("No scores were found!"), true);
                    
                    plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
                });
                return;
            }

            auto const& metadata = result["metadata"].GetObject();
            int perPage = metadata["itemsPerPage"].GetInt();
            int pageNum = metadata["page"].GetInt();
            int total = metadata["total"].GetInt();
            plvc->scores->Clear();

            for (int index = 0; index < 10; ++index)
            {
                if (index < (int)scores.Size())
                {
                    auto const& score = scores[index];
                    
                    Score currentScore = Score(score);
                    scoreVector[index] = currentScore;
                    
                    if (currentScore.playerId.compare(PlayerController::currentPlayer->id) == 0) {
                        selectedScore = index;
                    }

                    LeaderboardTableView::ScoreData* scoreData = LeaderboardTableView::ScoreData::New_ctor(
                        currentScore.modifiedScore, 
                        il2cpp_utils::createcsstr(generateLabel(currentScore)), 
                        currentScore.rank, 
                        false);
                    plvc->scores->Add(scoreData);
                }
            }
                
            plvc->leaderboardTableView->scores = plvc->scores;
            plvc->leaderboardTableView->specialScorePos = 10;
            QuestUI::MainThreadScheduler::Schedule([pageNum, perPage, total] {
                if (sspageUpButton != NULL) {
                    sspageDownButton->set_interactable(pageNum != 1);
                    sspageUpButton->set_interactable(pageNum * perPage < total);
                } else if (upPageButton != NULL) {
                    upPageButton->get_gameObject()->SetActive(pageNum != 1);
                    downPageButton->get_gameObject()->SetActive(pageNum * perPage < total);
                }

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

        plvc->loadingControl->ShowText(il2cpp_utils::createcsstr("Loading"), true);
    }

    static UnityEngine::Color selectedColor = UnityEngine::Color(0.0, 0.4, 1.0, 1.0);
    static UnityEngine::Color fadedColor = UnityEngine::Color(0.8, 0.8, 0.8, 0.2);
    static UnityEngine::Color fadedHoverColor = UnityEngine::Color(0.5, 0.5, 0.5, 0.2);

    void updateModifiersButton() {
        modifiersButton->set_defaultColor(modifiers ? selectedColor : fadedColor);
        modifiersButton->set_highlightColor(modifiers ? selectedColor : fadedHoverColor);

        if (modifiers) {
            modifiersButtonHover->set_text(il2cpp_utils::createcsstr("Show leaderboard without positive modifiers"));
        } else {
            modifiersButtonHover->set_text(il2cpp_utils::createcsstr("Show leaderboard with positive modifiers"));
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
        selectedScore = 10;
        if (plvc->leaderboardTableView->scores != NULL) {
            plvc->leaderboardTableView->scores->Clear();
        }
        plvc->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)plvc->leaderboardTableView, true);
    }

    void PageUp() {
        page++;

        clearTable();
        refreshFromTheServer();
    }

    void PageDown() {
        if (page > 1) {
            page--;
        }

        clearTable();
        refreshFromTheServer();
    }

    void updateLeaderboard(PlatformLeaderboardViewController* self) {
        clearTable();
        page = 1;
        isLocal = false;

        if (PlayerController::currentPlayer == std::nullopt) {
            self->loadingControl->ShowText(il2cpp_utils::createcsstr("Please sign up or log in mod settings!"), true);
            return;
        }

        if (!bundleLoaded) {
            self->StartCoroutine(reinterpret_cast<System::Collections::IEnumerator*>(custom_types::Helpers::CoroutineHelper::New(BundleLoader::LoadBundle())));
            bundleLoaded = true;
        }
        
        if (uploadStatus == NULL) {
            if (!ssInstalled) {
                Array<::HMUI::IconSegmentedControl::DataItem*>* dataItems = Array<::HMUI::IconSegmentedControl::DataItem*>::NewLength(4);
                Array<PlatformLeaderboardsModel::ScoresScope>* scoreScopes = Array<PlatformLeaderboardsModel::ScoresScope>::NewLength(4);
                for (int index = 0; index < 3; ++index)
                {
                    dataItems->values[index] = plvc->scopeSegmentedControl->dataItems->get(index);
                    scoreScopes->values[index] = plvc->scoreScopes->get(index);
                }
                dataItems->values[3] = HMUI::IconSegmentedControl::DataItem::New_ctor(plvc->friendsLeaderboardIcon, il2cpp_utils::createcsstr("Country"));
                scoreScopes->values[3] = PlatformLeaderboardsModel::ScoresScope(3);

                plvc->scopeSegmentedControl->SetData(dataItems);
                plvc->scoreScopes = scoreScopes;
            }

            parentScreen = CreateCustomScreen(plvc, UnityEngine::Vector2(480, 160), plvc->screen->get_transform()->get_position(), 140);

            BeatLeader::initScoreDetailsPopup(&scoreDetailsUI, self->get_transform());
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

            auto websiteLink = CreateClickableImage(parentScreen->get_transform(), BundleLoader::beatLeaderLogoGradient, UnityEngine::Vector2(100, 50), UnityEngine::Vector2(12, 12), []() {
                string url = WebUtils::WEB_URL;
                if (PlayerController::currentPlayer != std::nullopt) {
                    url += "u/" + PlayerController::currentPlayer->id;
                }
                UnityEngine::Application::OpenURL(il2cpp_utils::createcsstr(url));
            });
            logoAnimation = websiteLink->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
            logoAnimation->Init(websiteLink);

            if (retryButton) UnityEngine::GameObject::Destroy(retryButton);
            retryButton = ::QuestUI::BeatSaberUI::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(105, 63), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                showRetryButton = false;
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

            if (!ssInstalled) {
                upPageButton = CreateClickableImage(parentScreen->get_transform(), Sprites::get_UpIcon(), UnityEngine::Vector2(100, 17), UnityEngine::Vector2(8, 5.12), [](){
                    page--;
                    clearTable();
                    refreshFromTheServer();
                });
                downPageButton = CreateClickableImage(parentScreen->get_transform(), Sprites::get_DownIcon(), UnityEngine::Vector2(100, -17), UnityEngine::Vector2(8, 5.12), [](){
                    page++;
                    clearTable();
                    refreshFromTheServer();
                });
            }

            modifiersButton = CreateClickableImage(parentScreen->get_transform(), BundleLoader::modifiersIcon, UnityEngine::Vector2(100, 28), UnityEngine::Vector2(4, 4), [](){
                modifiers = !modifiers;
                getModConfig().Modifiers.SetValue(modifiers);
                clearTable();
                updateModifiersButton();
                refreshFromTheServer();
            });
            modifiers = getModConfig().Modifiers.GetValue();
            modifiersButtonHover = ::QuestUI::BeatSaberUI::AddHoverHint(modifiersButton->get_gameObject(), "Show leaderboard without positive modifiers");
            updateModifiersButton();

            auto votingButtonImage = CreateClickableImage(parentScreen->get_transform(), BundleLoader::modifiersIcon, UnityEngine::Vector2(100, 22), UnityEngine::Vector2(4, 4), []() {
                if (votingButton->state != 2) return;
                
                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            votingButton = websiteLink->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            votingButton->Init(votingButtonImage);

            if (PlayerController::currentPlayer != std::nullopt) {
                updatePlayerInfoLabel();
            }

            if (ssInstalled) {
                Array<UnityEngine::UI::Button*> *buttons = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::UI::Button*>();
                for (size_t i = 0; i < buttons->Length(); i++)
                {
                    auto button = buttons->get(i);

                    TMPro::TextMeshProUGUI* textMesh = button->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
                    if (textMesh && to_utf8(csstrtostr(textMesh->get_text())) == "") {
                        auto position = button->GetComponent<UnityEngine::RectTransform *>()->get_anchoredPosition();
                        if (position.x == -40 && position.y == 20) {
                            ssDownAction = button->get_onClick();
                            blDownAction = UnityEngine::UI::Button::ButtonClickedEvent::New_ctor();
                            sspageDownButton = button;

                            auto delegate = il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction*>(classof(UnityEngine::Events::UnityAction*), button, PageDown);
                            blDownAction->AddListener(delegate);
                            button->set_onClick(blDownAction);
                        } else if (position.x == -40 && position.y == -20) {
                            ssUpAction = button->get_onClick();
                            blUpAction = UnityEngine::UI::Button::ButtonClickedEvent::New_ctor();
                            sspageUpButton = button;

                            auto delegate = il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction*>(classof(UnityEngine::Events::UnityAction*), button, PageUp);
                            blUpAction->AddListener(delegate);
                            button->set_onClick(blUpAction);
                        }
                    }
                }
            }
        }
        
        if (ssInstalled && ssElements.size() < 10) {
            bool wasEmpty = ssElements.size() == 0;
            Array<UnityEngine::Transform*> *transforms = plvc->get_gameObject()->get_transform()->FindObjectsOfType<UnityEngine::Transform*>();
            for (size_t i = 0; i < transforms->Length(); i++)
            {
                auto transform = transforms->get(i);
                auto name =  transform->get_name();
                if (name != NULL && (to_utf8(csstrtostr(name)) == "ScoreSaberClickableImage" 
                                || (wasEmpty && to_utf8(csstrtostr(name)) == "QuestUIHorizontalLayoutGroup"))) {
                    transform->get_gameObject()->SetActive(false);
                    ssElements.push_back(transform);
                }
            }
        }

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(false);
            downPageButton->get_gameObject()->SetActive(false);
        }

        IPreviewBeatmapLevel* levelData = reinterpret_cast<IPreviewBeatmapLevel*>(self->difficultyBeatmap->get_level());
        if (!to_utf8(csstrtostr(levelData->get_levelID())).starts_with("custom_level")) {
            self->loadingControl->Hide();
            self->hasScoresData = false;
            self->loadingControl->ShowText(il2cpp_utils::createcsstr("Leaderboards for this map are not supported!"), false);
            self->leaderboardTableView->tableView->SetDataSource((HMUI::TableView::IDataSource *)self->leaderboardTableView, true);
        } else {
            refreshFromTheServer();
        }
    }

    void updateSelectedLeaderboard() {
        if (uploadStatus != NULL) {
            parentScreen->get_gameObject()->SetActive(showBeatLeader);
            retryButton->get_gameObject()->SetActive(showBeatLeader && showRetryButton);

            LevelInfoUI::SetLevelInfoActive(showBeatLeader);
            ModifiersUI::SetModifiersActive(showBeatLeader);

            for (size_t i = 0; i < ssElements.size(); i++)
            {
                ssElements[i]->get_gameObject()->SetActive(!showBeatLeader);
            }

            if (showBeatLeader) {
                blDownAction = UnityEngine::UI::Button::ButtonClickedEvent::New_ctor();

                auto delegate = il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction*>(classof(UnityEngine::Events::UnityAction*), sspageDownButton, PageDown);
                blDownAction->AddListener(delegate);

                blUpAction = UnityEngine::UI::Button::ButtonClickedEvent::New_ctor();

                auto delegate2 = il2cpp_utils::MakeDelegate<UnityEngine::Events::UnityAction*>(classof(UnityEngine::Events::UnityAction*), sspageUpButton, PageUp);
                blUpAction->AddListener(delegate2);
            }

            if (sspageUpButton != NULL) {
                sspageUpButton->set_onClick(showBeatLeader ? blUpAction : ssUpAction);
                sspageDownButton->set_onClick(showBeatLeader ? blDownAction : ssDownAction);
                sspageDownButton->set_interactable(!showBeatLeader);
                sspageUpButton->set_interactable(!showBeatLeader);
            }
            
            HMUI::ImageView* imageView = plvc->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
            
            if (showBeatLeader) {
                imageView->set_color(UnityEngine::Color(0.64,0.64,0.64,1));
                imageView->set_color0(UnityEngine::Color(0.93,0,0.55,1));
                imageView->set_color1(UnityEngine::Color(0.25,0.52,0.9,1));
            } else {
                imageView->set_color(UnityEngine::Color(0.5,0.5,0.5,1));
                imageView->set_color0(UnityEngine::Color(0.5,0.5,0.5,1));
                imageView->set_color1(UnityEngine::Color(0.5,0.5,0.5,1));
            }
        }
    }

    void refreshLeaderboardCall(PlatformLeaderboardViewController* self) {
        if (showBeatLeader) {
            updateLeaderboard(self);
        }

        if (ssInstalled && showBeatLeaderButton == NULL) {
            showBeatLeaderButton = ::QuestUI::BeatSaberUI::CreateToggle(self->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_transform(), "Show BL", showBeatLeader, UnityEngine::Vector2(0, 0), [](bool changed){
                showBeatLeader = !showBeatLeader;
                getModConfig().ShowBeatleader.SetValue(showBeatLeader);
                plvc->Refresh(true, true);
                updateSelectedLeaderboard();
            });
            resize(showBeatLeaderButton, 170, 0);

            // if (!showBeatLeader) {
            //     QuestUI::MainThreadScheduler::Schedule([] {
            //         HMUI::ImageView* imageView = plvc->get_gameObject()->get_transform()->Find(il2cpp_utils::createcsstr("HeaderPanel"))->get_gameObject()->GetComponentInChildren<HMUI::ImageView*>();
            //         imageView->set_color(UnityEngine::Color(0.5,0.5,0.5,1));
            //         imageView->set_color0(UnityEngine::Color(0.5,0.5,0.5,1));
            //         imageView->set_color1(UnityEngine::Color(0.5,0.5,0.5,1));
            //     });
            // }
            
            updateSelectedLeaderboard();
        }
    }

    MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool showLoadingIndicator, bool clear) {
        plvc = self;
        if (!showBeatLeader) {
            RefreshLeaderboard(self, showLoadingIndicator, clear);
            ssWasOpened = true;
        }

        if (ssInstalled && sspageUpButton == NULL) {
            QuestUI::MainThreadScheduler::Schedule([self] {
                refreshLeaderboardCall(self);
            });
        } else {
            refreshLeaderboardCall(self);
        }
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, HMUI::TableCell*, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = (LeaderboardTableCell *)LeaderboardCellSource(self, tableView, row);

        if (showBeatLeader) {
            if (!isLocal && result->scoreText->get_fontSize() != 2) {
                result->playerNameText->set_enableAutoSizing(false);
                result->playerNameText->set_richText(true);
                
                resize(result->playerNameText, 13, 0);
                move(result->playerNameText, -2, 0);
                move(result->fullComboText, 0.2, 0);
                move(result->scoreText, 4, 0);
                result->playerNameText->set_fontSize(3);
                result->fullComboText->set_fontSize(3);
                result->scoreText->set_fontSize(2);
            }

            if (!isLocal && !cellBackgrounds.count(result)) {
                EmojiSupport::AddSupport(result->playerNameText);

                avatars[result] = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), plvc->aroundPlayerLeaderboardIcon, UnityEngine::Vector2(-32, 0), UnityEngine::Vector2(4, 4));

                auto scoreSelector = CreateClickableImage(result->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6), [result]() {
                    scoreDetailsUI->modal->Show(true, true, nullptr);

                    scoreDetailsUI->setScore(cellScores[result]);
                });
                scoreSelector->set_material(UnityEngine::Object::Instantiate(BundleLoader::scoreUnderlineMaterial));
                
                cellHighlights[result] = scoreSelector;

                auto backgroundImage = ::QuestUI::BeatSaberUI::CreateImage(result->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(0, 0), UnityEngine::Vector2(80, 6));
                backgroundImage->set_material(BundleLoader::scoreBackgroundMaterial);
                backgroundImage->get_transform()->SetAsFirstSibling();
                cellBackgrounds[result] = backgroundImage;  

                // auto tagsList = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(result->get_transform());
                // clanGroups[result] = tagsList;         
            }
        } else {
            if (result->scoreText->get_fontSize() == 2) {
                result->playerNameText->set_enableAutoSizing(true);
                resize(result->playerNameText, -13, 0);
                move(result->playerNameText, 2, 0);
                move(result->fullComboText, -0.2, 0);
                move(result->scoreText, -4, 0);
                result->playerNameText->set_fontSize(4);
                result->fullComboText->set_fontSize(4);
                result->scoreText->set_fontSize(4);
            }
        }
        

        if (!isLocal && showBeatLeader) {
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

    void updateStatus(ReplayUploadStatus status, string description, float progress) {
        if (uploadStatus != NULL && showBeatLeader) {
            uploadStatus->SetText(il2cpp_utils::createcsstr(description));
            switch (status)
            {
                case ReplayUploadStatus::finished:
                    logoAnimation->SetAnimating(false);
                    updateVotingButton(lastVotingStatusUrl);
                    plvc->Refresh(true, true);
                    break;
                case ReplayUploadStatus::error:
                    logoAnimation->SetAnimating(false);
                    retryButton->get_gameObject()->SetActive(true);
                    showRetryButton = true;
                    break;
                case ReplayUploadStatus::inProgress:
                    logoAnimation->SetAnimating(true);
                    if (progress >= 100)
                        uploadStatus->SetText(il2cpp_utils::createcsstr("<color=#b103fcff>Posting replay: Finishing up..."));
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
        INSTALL_HOOK(logger, LocalLeaderboardDidActivate);

        PlayerController::playerChanged.emplace_back([](std::optional<Player> const& updated) {
            QuestUI::MainThreadScheduler::Schedule([] {
                if (playerName != NULL) {
                    updatePlayerInfoLabel();
                }
            });
        });

        ssInstalled = false;
        showBeatLeader = true;

        QuestUI::MainThreadScheduler::Schedule([] {
            LoggerContextObject logger = getLogger().WithContext("load");
            INSTALL_HOOK(logger, RefreshLeaderboard);
            INSTALL_HOOK(logger, LeaderboardCellSource);

            FILE *fp = fopen("/sdcard/BMBFData/config.json", "r");
            rapidjson::Document config;
            if (fp != NULL) {
                char buf[0XFFFF];
                rapidjson::FileReadStream input(fp, buf, sizeof(buf));
                config.ParseStream(input);
                fclose(fp);
            }

            if (!config.HasParseError() && config.IsObject()) {
                auto mods = config["Mods"].GetArray();
                for (int index = 0; index < (int)mods.Size(); ++index)
                {
                    auto const& mod = mods[index];
                    
                    if (strcmp(mod["Id"].GetString(), "scoresaber") == 0 && mod["Installed"].GetBool()) {
                        ssInstalled = true;
                        showBeatLeader = getModConfig().ShowBeatleader.GetValue();
                        break;
                    }
                }
            }
        });
    }

    void reset() {
        uploadStatus = NULL;
        plvc = NULL;
        scoreDetailsUI = NULL;
        votingUI = NULL;
        cellScores.clear();
        avatars = {};
        cellHighlights = {};
        cellBackgrounds = {};
        bundleLoaded = false;
        showBeatLeaderButton = NULL;
        ssWasOpened = false;
        if (ssInstalled) {
            showBeatLeader = getModConfig().ShowBeatleader.GetValue();
        }
        ssElements = vector<UnityEngine::Transform*>();
        ModifiersUI::ResetModifiersUI();
    }    
}