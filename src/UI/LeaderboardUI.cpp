#include "include/UI/LeaderboardUI.hpp"

#include "include/Models/Replay.hpp"
#include "include/Models/Score.hpp"
#include "include/Models/ClanScore.hpp"
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

#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/ModConfig.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"

#include "bsml/shared/bsml.hpp"
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
    BeatLeader::PlayerAvatar* playerAvatar = NULL;
    UnityEngine::UI::Toggle* showBeatLeaderButton = NULL;

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

    static string lastUrl = "";
    static string lastVotingStatusUrl = "";
    static string votingUrl = "";

    static ReplayUploadStatus cachedStatus;
    static string cachedDescription;
    static float cachedProgress; 
    static bool cachedShowRestart;
    static bool statusWasCached;
    static vector<UnityEngine::Transform*> ssElements;
    bool ssInstalled = true;
    bool ssWasOpened = false;
    bool showBeatLeader = false;
    bool restoredFromPreferences = false;

    UnityEngine::UI::Button* sspageUpButton;
    UnityEngine::UI::Button* sspageDownButton;

    static map<Context, string> contextToUrlString = {
        {Context::Standard, "modifiers"},
        {Context::NoMods, "standard"},
        {Context::NoPause, "nopause"},
        {Context::Golf, "golf"},
        {Context::SCPM, "scpm"},
    };

    static map<Context, string> contextToDisplayString = {
        {Context::Standard, "General"},
        {Context::NoMods, "No Mods"},
        {Context::NoPause, "No Pauses"},
        {Context::Golf, "Golf"},
        {Context::SCPM, "SCPM"},
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

    void updatePlayerInfoLabel() {
        auto const& player = PlayerController::currentPlayer;
        if (player != std::nullopt) {
            if (!player->name.empty()) {

                updatePlayerRank();
                
                playerName->set_alignment(TMPro::TextAlignmentOptions::Center);
                playerName->SetText(FormatUtils::FormatNameWithClans(PlayerController::currentPlayer.value(), 25, false), true);

                auto params = GetAvatarParams(player.value(), false);
                playerAvatar->SetPlayer(player->avatar, params.baseMaterial, params.hueShift, params.saturation);
                
                if (plvc != NULL) {
                    auto sprite = BundleLoader::bundle->GetCountryIcon(player->country);
                    if (!ssInstalled) {
                        auto countryControl = plvc->_scopeSegmentedControl->_dataItems.get(3);
                        countryControl->set_hintText("Country");

                        plvc->_scopeSegmentedControl->_dataItems.get(3)->set_icon(sprite);
                        plvc->_scopeSegmentedControl->SetData(plvc->_scopeSegmentedControl->_dataItems);
                    }

                    if (countryRankIcon) {
                        countryRankIcon->set_sprite(sprite);
                        RectTransform* rectTransform = countryRankIcon->get_transform().cast<RectTransform>();
                        rectTransform->set_sizeDelta({3.2, sprite->get_bounds().get_size().y * 10});
                    }
                }

            } else {
                playerName->SetText(player->name + ", play something!", true);
            }
        } else {
            globalRank->SetText("#0", true);
            countryRankAndPp->SetText("#0", true);
            playerAvatar->HideImage();
            if (countryRankIcon) {
                countryRankIcon->set_sprite(BundleLoader::bundle->globeIcon);
            }
            playerName->SetText("", true);
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
        }
    }

    MAKE_HOOK_MATCH(
        SegmentedControlHandleCellSelection, 
        &SegmentedControl::HandleCellSelectionDidChange,
        void, 
        SegmentedControl* self,
        ::HMUI::SelectableCell* selectableCell, 
        ::HMUI::__SelectableCell__TransitionType transitionType, 
        ::System::Object* changeOwner) {
        SegmentedControlHandleCellSelection(self, selectableCell, transitionType, changeOwner);

        if (plvc &&
            leaderboardLoaded &&
            self == plvc->_scopeSegmentedControl.ptr()) {
            cachedSelector = -1;
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

    void refreshFromTheServerScores() {
        auto [hash, difficulty, mode] = getLevelDetails(plvc->_beatmapKey);
        string url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode + "/" + contextToUrlString[static_cast<Context>(getModConfig().Context.GetValue())];

        int selectedCellNumber = cachedSelector != -1 ? cachedSelector : plvc->_scopeSegmentedControl->selectedCellNumber;

        switch (selectedCellNumber)
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
            case Context::SCPM:
                modifiersIcon = BundleLoader::bundle->scpmIcon;
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
                bool header = (name == "bsmlHorizontalLayoutGroup" || name == "bsmlVerticalLayoutGroup") &&
                            transform->get_parent() && transform->get_parent()->get_parent() &&
                            transform->get_parent()->get_parent()->get_name() == "PlatformLeaderboardViewController";
                if (infoIcon || header) {
                    transform->get_gameObject()->SetActive(false);
                    ssElements.push_back(transform);
                }
            }
        }

        if (PlayerController::currentPlayer == std::nullopt) {
            self->_loadingControl->Hide();
            
            if (preferencesButton == NULL) {
                loginPrompt = ::BSML::Lite::CreateText(plvc->get_transform(), "Please sign up or log in to post scores!", false, UnityEngine::Vector2(4, 10));
                preferencesButton = ::BSML::Lite::CreateUIButton(plvc->get_transform(), "Open settings", UnityEngine::Vector2(0, 0), [](){
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
            if (!ssInstalled) {
                ArrayW<::HMUI::IconSegmentedControl::DataItem*> dataItems = ArrayW<::HMUI::IconSegmentedControl::DataItem*>(4);
                ArrayW<PlatformLeaderboardsModel::ScoresScope> scoreScopes = ArrayW<PlatformLeaderboardsModel::ScoresScope>(4);
                for (int index = 0; index < 3; ++index)
                {
                    dataItems[index] = self->_scopeSegmentedControl->_dataItems.get(index);
                    scoreScopes[index] = self->_scoreScopes.get(index);
                }
                dataItems[3] = HMUI::IconSegmentedControl::DataItem::New_ctor(self->_friendsLeaderboardIcon, "Country");
                scoreScopes[3] = PlatformLeaderboardsModel::ScoresScope(3);

                plvc->_scopeSegmentedControl->SetData(dataItems);
                plvc->_scoreScopes = scoreScopes;
            }

            parentScreen = CreateCustomScreen(self, UnityEngine::Vector2(480, 160), self->screen->get_transform()->get_position(), 140);
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

            playerName = ::BSML::Lite::CreateText(parentScreen->get_transform(), "", false, UnityEngine::Vector2(140, 53), UnityEngine::Vector2(60, 10));
            playerName->set_fontSize(6);

            EmojiSupport::AddSupport(playerName);

            auto rankLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(parentScreen->get_transform());
            rankLayout->set_spacing(3);
            EnableHorizontalFit(rankLayout);
            auto rectTransform = rankLayout->get_transform().cast<RectTransform>();
            rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
            rectTransform->set_anchoredPosition({138, 45});

            auto globalLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            globalLayout->set_spacing(1);
            EnableHorizontalFit(globalLayout);
            globalRankIcon = ::BSML::Lite::CreateImage(globalLayout->get_transform(), BundleLoader::bundle->globeIcon);
            globalRank = ::BSML::Lite::CreateText(globalLayout->get_transform(), "", false);

            auto countryLayout = ::BSML::Lite::CreateHorizontalLayoutGroup(rankLayout->get_transform());
            countryLayout->set_spacing(1);
            EnableHorizontalFit(countryLayout);
            countryRankIcon = ::BSML::Lite::CreateImage(countryLayout->get_transform(), BundleLoader::bundle->globeIcon);
            countryRankAndPp = ::BSML::Lite::CreateText(countryLayout->get_transform(), "", false);
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

            if (retryButton) UnityEngine::GameObject::Destroy(retryButton);
            retryButton = ::BSML::Lite::CreateUIButton(parentScreen->get_transform(), "Retry", UnityEngine::Vector2(105, 63), UnityEngine::Vector2(15, 8), [](){
                retryButton->get_gameObject()->SetActive(false);
                showRetryButton = false;
                retryCallback();
            });
            retryButton->get_gameObject()->SetActive(false);
            retryButton->GetComponentInChildren<CurvedTextMeshPro*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

            if(uploadStatus) UnityEngine::GameObject::Destroy(uploadStatus);
            uploadStatus = ::BSML::Lite::CreateText(parentScreen->get_transform(), "", false);
            move(uploadStatus, 150, 60);
            resize(uploadStatus, 10, 0);
            uploadStatus->set_fontSize(3);
            uploadStatus->set_richText(true);
            upPageButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), Sprites::get_UpIcon(), [](){
                PageUp();
            }, UnityEngine::Vector2(100, 17), UnityEngine::Vector2(8, 5.12));
            downPageButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), Sprites::get_DownIcon(), [](){
                PageDown();
            }, UnityEngine::Vector2(100, -20), UnityEngine::Vector2(8, 5.12));

            contextsButton = ::BSML::Lite::CreateClickableImage(parentScreen->get_transform(), BundleLoader::bundle->modifiersIcon, [](){
                contextsContainer->Show(true, true, nullptr);
            }, UnityEngine::Vector2(100, 28), UnityEngine::Vector2(6, 6));
            contextsButton->set_defaultColor(SelectedColor);
            contextsButton->set_highlightColor(SelectedColor);
            // We need to add an empty hover hint, so we can set it later to the correct content depending on the selected context
            contextsButtonHover = ::BSML::Lite::AddHoverHint(contextsButton, "");
            initContextsModal(self->get_transform());
            updateModifiersButton();

            auto votingButtonImage = ::BSML::Lite::CreateClickableImage(
                parentScreen->get_transform(), 
                BundleLoader::bundle->modifiersIcon, 
                []() {
                    if (votingButton->state != 2) return;
                    
                    votingUI->reset();
                    votingUI->modal->Show(true, true, nullptr);
                },
                UnityEngine::Vector2(100, 22), 
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
            
            CaptorClanUI::initCaptorClan(plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject(), plvc->get_gameObject()->get_transform()->Find("HeaderPanel")->get_gameObject()->GetComponentInChildren<TMPro::TextMeshProUGUI*>());
            CaptorClanUI::showClanRankingCallback = []() {
                clearTable();
                refreshFromTheServerCurrent();
            };
        }

        if (ssInstalled && !sspageUpButton) {
            ArrayW<UnityEngine::UI::Button*> buttons = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::UI::Button*>();
            for (size_t i = 0; i < buttons.size(); i++)
            {
                auto button = buttons[i];

                TMPro::TextMeshProUGUI* textMesh = button->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
                if (textMesh && textMesh->get_text() && textMesh->get_text() == "") {
                    auto position = button->GetComponent<UnityEngine::RectTransform *>()->get_anchoredPosition();
                    if (position.x == -40 && position.y == 20) {
                        sspageDownButton = button;
                        sspageDownButton->get_gameObject()->SetActive(false);
                    } else if (position.x == -40 && position.y == -20) {
                        sspageUpButton = button;
                        sspageUpButton->get_gameObject()->SetActive(false);
                    }
                }
            }
        }

        if (upPageButton != NULL) {
            upPageButton->get_gameObject()->SetActive(false);
            downPageButton->get_gameObject()->SetActive(false);
        }

        refreshFromTheServerCurrent();
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
        CaptorClanUI::setActive(showBeatLeader && getModConfig().CaptureActive.GetValue());

        for (size_t i = 0; i < ssElements.size(); i++)
        {
            ssElements[i]->get_gameObject()->SetActive(!showBeatLeader);
        }

        if (sspageUpButton != NULL) {
            sspageDownButton->get_gameObject()->SetActive(!showBeatLeader);
            sspageUpButton->get_gameObject()->SetActive(!showBeatLeader);
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

        if (ssInstalled && showBeatLeaderButton == NULL) {
            auto headerTransform = self->get_gameObject()->get_transform()->Find("HeaderPanel")->get_transform();
            BSML::Lite::CreateText(headerTransform, "BeatLeader", {-12.2, 2});
            showBeatLeaderButton = CreateToggle(headerTransform, showBeatLeader, UnityEngine::Vector2(-84.5, 0), [](bool changed){
                showBeatLeader = !showBeatLeader;
                getModConfig().ShowBeatleader.SetValue(showBeatLeader);
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
            
            updateSelectedLeaderboard();
        }
    }

    MAKE_HOOK_MATCH(RefreshLeaderboard, &PlatformLeaderboardViewController::Refresh, void, PlatformLeaderboardViewController* self, bool showLoadingIndicator, bool clear) {
        plvc = self;
        if (!showBeatLeader) {
            RefreshLeaderboard(self, showLoadingIndicator, clear);
            ssWasOpened = true;
        }

        if (ssInstalled && showBeatLeader && sspageUpButton == NULL) {
            std::async(std::launch::async, [] () {
                std::this_thread::sleep_for(std::chrono::seconds{1});
                BSML::MainThreadScheduler::Schedule([] {
                    refreshLeaderboardCall();
                });
            });
        } else {
            refreshLeaderboardCall();
        }

        leaderboardLoaded = true;
    }

    LeaderboardTableCell* CellForIdxReimplement(LeaderboardTableView* self, HMUI::TableView* tableView) {
        LeaderboardTableCell* leaderboardTableCell = tableView->DequeueReusableCellForIdentifier("Cell").try_cast<LeaderboardTableCell>().value_or(UnityW<LeaderboardTableCell>());
        if (leaderboardTableCell == NULL)
        {
            leaderboardTableCell = (LeaderboardTableCell *)Object::Instantiate<LeaderboardTableCell*>(self->_cellPrefab);
            ((TableCell *)leaderboardTableCell)->set_reuseIdentifier("Cell");
        }
        auto score = self->_scores->get_Item(10);
        leaderboardTableCell->set_rank(score->rank);
        leaderboardTableCell->set_playerName(score->playerName);
        leaderboardTableCell->set_score(score->score);
        leaderboardTableCell->set_showFullCombo(score->fullCombo);
        leaderboardTableCell->set_showSeparator(false);
        leaderboardTableCell->set_specialScore(self->_specialScorePos == 10);
        return leaderboardTableCell;
    }

    MAKE_HOOK_MATCH(LeaderboardCellSource, &LeaderboardTableView::CellForIdx, UnityW<HMUI::TableCell>, LeaderboardTableView* self, HMUI::TableView* tableView, int row) {
        LeaderboardTableCell* result = row == 10 ? CellForIdxReimplement(self, tableView) : LeaderboardCellSource(self, tableView, row).cast<LeaderboardTableCell>().ptr();

        if (showBeatLeader && !isLocal) {
            if (result->_playerNameText->get_fontSize() > 3 || result->_playerNameText->get_enableAutoSizing()) {
                result->_playerNameText->set_enableAutoSizing(false);
                result->_playerNameText->set_richText(true);
                
                resize(result->_playerNameText, 24, 0);
                move(result->_rankText, 1, 0);
                result->_rankText->set_alignment(TMPro::TextAlignmentOptions::Right);

                move(result->_playerNameText, -0.5, 0);
                move(result->_fullComboText, 0.2, 0);
                move(result->_scoreText, 4, 0);
                result->_playerNameText->set_fontSize(3);
                result->_fullComboText->set_fontSize(3);
                result->_scoreText->set_fontSize(2);
                EmojiSupport::AddSupport(result->_playerNameText);

                if (!cellBackgrounds.count(result)) {
                    avatars[result] = ::BSML::Lite::CreateImage(result->get_transform(), plvc->_aroundPlayerLeaderboardIcon, UnityEngine::Vector2(-30, 0), UnityEngine::Vector2(4, 4));
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
                move(result->_rankText, 6.2, 0.1);
                result->_rankText->set_alignment(TMPro::TextAlignmentOptions::Left);
                move(result->_playerNameText, 0.5, 0);
                move(result->_fullComboText, -0.2, 0);
                move(result->_scoreText, -4, 0);
                result->_playerNameText->set_fontSize(4);
                result->_fullComboText->set_fontSize(4);
                result->_scoreText->set_fontSize(4);
            }
        }
        

        if (!isLocal && showBeatLeader) {
            if (cellBackgrounds.count(result)) {
                if (!CaptorClanUI::showClanRanking) {
                    auto player = scoreVector[row].player;
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
                    cellScores[result] = scoreVector[row];

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
                    
                    if (PlayerController::InClan(clan.tag)) {
                        cellBackgrounds[result]->set_color(ownScoreColor);
                    } else {
                        cellBackgrounds[result]->set_color(someoneElseScoreColor);
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

    void initContextsModal(UnityEngine::Transform* parent){
        auto container = BSML::Lite::CreateModal(parent, {40, static_cast<float>((static_cast<int>(Context::SCPM) + 2) * 10 + 5)}, nullptr, true);

        BSML::Lite::CreateText(container->get_transform(), "Scores Context", {20, 19});

        for(int i = 0; i <= static_cast<int>(Context::SCPM); i++)
        {
            BSML::Lite::CreateUIButton(container->get_transform(), contextToDisplayString[static_cast<Context>(i)], {0.0f, static_cast<float>(21 - (i + 1) * 10)}, [i](){
                // Set the new value
                getModConfig().Context.SetValue(i);
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
        }

        contextsContainer = container;
    }

    void initSettingsModal(UnityEngine::Transform* parent){
        auto container = BSML::Lite::CreateModal(parent, {40,60}, nullptr, true);
        
        BSML::Lite::CreateText(container->get_transform(), "Leaderboard Settings", {16, 24});

        BSML::Lite::CreateText(container->get_transform(), "Avatar", {12, 14});

        CreateToggle(container->get_transform(), getModConfig().AvatarsActive.GetValue(), {-3, 16}, [](bool value){
            getModConfig().AvatarsActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Clans", {12, 4});

        CreateToggle(container->get_transform(), getModConfig().ClansActive.GetValue(), {-3, 6}, [](bool value){
            getModConfig().ClansActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Score", {12, -6});

        CreateToggle(container->get_transform(), getModConfig().ScoresActive.GetValue(), {-3, -4}, [](bool value){
            getModConfig().ScoresActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Time", {12, -16});

        CreateToggle(container->get_transform(), getModConfig().TimesetActive.GetValue(), {-3, -14}, [](bool value){
            getModConfig().TimesetActive.SetValue(value);
            plvc->Refresh(true, true);
        });

        BSML::Lite::CreateText(container->get_transform(), "Capture", {12, -26});

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
        static SafePtrUnity<UnityEngine::UI::Toggle> toggleCopy;
        if(!toggleCopy){
            toggleCopy = Resources::FindObjectsOfTypeAll<UnityEngine::UI::Toggle*>()->FirstOrDefault([](auto x) {return x->get_transform()->get_parent()->get_gameObject()->get_name() == "Fullscreen"; });
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

    MAKE_HOOK_MATCH(LocalLeaderboardDidActivate, &LocalLeaderboardViewController::DidActivate, void, LocalLeaderboardViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        isLocal = true;

        LocalLeaderboardDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    }

    void setup() {
        if (hooksInstalled) return;

        INSTALL_HOOK(BeatLeaderLogger, LeaderboardActivate);
        INSTALL_HOOK(BeatLeaderLogger, LeaderboardDeactivate);
        INSTALL_HOOK(BeatLeaderLogger, LocalLeaderboardDidActivate);
        INSTALL_HOOK(BeatLeaderLogger, RefreshLeaderboard);
        INSTALL_HOOK(BeatLeaderLogger, LeaderboardCellSource);
        INSTALL_HOOK(BeatLeaderLogger, SegmentedControlHandleCellSelection);

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
        preferencesButton = NULL;
        parentScreen = NULL;
        sspageUpButton = NULL;
        cellScores.clear();
        avatars = {};
        cellHighlights = {};
        cellBackgrounds = {};
        showBeatLeaderButton = NULL;
        upPageButton = NULL;
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