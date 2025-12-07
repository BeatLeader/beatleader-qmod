#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "shared/Models/ScoreStats.hpp"

#include "include/UI/EmojiSupport.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/UI/Themes/ThemeUtils.hpp"
#include "include/UI/QuestUI.hpp"

#include "include/Core/ReplayPlayer.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "bsml/shared/BSML/Components/Backgroundable.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "main.hpp"

#include <sstream>
#include <filesystem>

using namespace BSML::Lite;
using namespace GlobalNamespace;

static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
static UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);
static UnityEngine::Color FadedHoverColor = UnityEngine::Color(0.5f, 0.5f, 0.5f, 0.2f);

static string replayLink;
static ScoreStats scoreStats;

static int scoreId;
static string platform;

static void MakeModalTransparent(HMUI::ModalView *modal) {
    int childCount = modal->get_transform()->get_childCount();
    for (int i = 0; i < childCount; i++) {
        auto* child = modal->get_transform()->GetChild(i)->GetComponent<RectTransform*>();

        if (child->get_gameObject()->get_name() == "BG") {
            child->GetComponent<UnityEngine::UI::Image*>()->set_sprite(Sprites::get_TransparentPixel());
        }
    }
}

void BeatLeader::initScoreDetailsPopup(
        BeatLeader::ScoreDetailsPopup** modalUIPointer, 
        UnityEngine::Transform* parent,
        function<void()> const &incognitoCallback){
    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::ScoreDetailsPopup*) malloc(sizeof(BeatLeader::ScoreDetailsPopup));
    modalUI->modal = QuestUI::CreateModal(parent, UnityEngine::Vector2(60, 90), {}, nullptr, true);
    MakeModalTransparent(modalUI->modal);

    auto modalTransform = modalUI->modal->get_transform();

    auto playerAvatarImage = ::BSML::Lite::CreateImage(modalTransform, NULL, UnityEngine::Vector2(0, 30), UnityEngine::Vector2(24, 24));
    modalUI->playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
    modalUI->playerAvatar->Init(playerAvatarImage);

    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, 18), UnityEngine::Vector2(58, 6));
    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, -3), UnityEngine::Vector2(60, 34));
    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, -24), UnityEngine::Vector2(36, 6));
    if (ReplayInstalled()) {
        UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(-24.5, -24), UnityEngine::Vector2(7, 7));
    }

    auto nameLevelLayoutGroup = BSML::Lite::CreateHorizontalLayoutGroup(modalTransform);
    nameLevelLayoutGroup->set_spacing(1);
    nameLevelLayoutGroup->set_padding(UnityEngine::RectOffset::New_ctor(1,1,1,1));
    nameLevelLayoutGroup->GetComponentInChildren<UnityEngine::UI::ContentSizeFitter*>()->set_horizontalFit(UnityEngine::UI::ContentSizeFitter_FitMode::PreferredSize);
    nameLevelLayoutGroup->GetComponentInChildren<UnityEngine::UI::LayoutElement *>()->set_preferredHeight(6.0f);
    nameLevelLayoutGroup->get_transform().cast<RectTransform>()->set_anchoredPosition({0, 18});
    modalUI->rank = QuestUI::CreateText(modalTransform, "", UnityEngine::Vector2(6.0, 16.0));
    auto prestigeGroup = BSML::Lite::CreateHorizontalLayoutGroup(nameLevelLayoutGroup->get_transform());;
    prestigeGroup->GetComponentInChildren<UnityEngine::UI::ContentSizeFitter*>()->set_horizontalFit(UnityEngine::UI::ContentSizeFitter_FitMode::PreferredSize);
    prestigeGroup->GetComponentInChildren<UnityEngine::UI::LayoutElement*>()->set_preferredWidth(4);
    prestigeGroup->GetComponentInChildren<UnityEngine::UI::LayoutElement *>()->set_preferredHeight(4);
    modalUI->prestigeIcon = BSML::Lite::CreateImage(prestigeGroup->get_transform(), BundleLoader::bundle->PrestigeIcon0, {0, 0}, {2, 2});
    modalUI->name = BSML::Lite::CreateText(nameLevelLayoutGroup->get_transform(), "");
    modalUI->sponsorMessage = QuestUI::CreateText(modalTransform, "", UnityEngine::Vector2(0, -32));

    EmojiSupport::AddSupport(modalUI->name);
    EmojiSupport::AddSupport(modalUI->sponsorMessage);

    modalUI->pp = QuestUI::CreateText(modalTransform, "", UnityEngine::Vector2(45.0, 16.0));

    modalUI->playerButtons.Setup(modalUI->modal, [modalUI, incognitoCallback](Player player) {
        modalUI->updatePlayerDetails(player);
        incognitoCallback();
    });
    modalUI->general = GeneralScoreDetails(modalUI->modal);
    modalUI->additional = AdditionalScoreDetails(modalUI->modal);
    modalUI->overview = ScoreStatsOverview(modalUI->modal);
    modalUI->grid = ScoreStatsGrid(modalUI->modal);
    modalUI->graph = ScoreStatsGraph(modalUI->modal);

    modalUI->generalButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->overview1Icon, [modalUI](){
        modalUI->selectTab(0);
    }, UnityEngine::Vector2(-14, -24), UnityEngine::Vector2(5, 5));
    ::BSML::Lite::AddHoverHint(modalUI->generalButton, "General score info");

    modalUI->additionalButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->overview2Icon, [modalUI](){
        modalUI->selectTab(1);
    }, UnityEngine::Vector2(-7, -24), UnityEngine::Vector2(5, 5));
    ::BSML::Lite::AddHoverHint(modalUI->generalButton, "General score info");

    modalUI->overviewButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->detailsIcon, [modalUI](){
        modalUI->selectTab(2);
    }, UnityEngine::Vector2(0, -24), UnityEngine::Vector2(5, 5));
    ::BSML::Lite::AddHoverHint(modalUI->overviewButton, "Detailed score info");

    modalUI->gridButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->gridIcon, [modalUI](){
        modalUI->selectTab(3);
    }, UnityEngine::Vector2(7, -24), UnityEngine::Vector2(5, 5));
    ::BSML::Lite::AddHoverHint(modalUI->gridButton, "Note accuracy distribution");

    modalUI->graphButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->graphIcon, [modalUI](){
        modalUI->selectTab(4);
    }, UnityEngine::Vector2(14, -24), UnityEngine::Vector2(5, 5));
    ::BSML::Lite::AddHoverHint(modalUI->graphButton, "Accuracy timeline graph");

    if (ReplayInstalled()) {
        modalUI->replayButton = ::BSML::Lite::CreateClickableImage(modalTransform, BundleLoader::bundle->replayIcon, [modalUI](){
            modalUI->playReplay();
        }, UnityEngine::Vector2(-24.5, -24), UnityEngine::Vector2(5, 5));
        ::BSML::Lite::AddHoverHint(modalUI->replayButton, "Watch the replay");
    }

    modalUI->setButtonsMaterial();

    modalUI->loadingText = QuestUI::CreateText(modalTransform, "Loading...", UnityEngine::Vector2(0.0, 0.0));
    modalUI->loadingText->set_alignment(TMPro::TextAlignmentOptions::Center);
    modalUI->loadingText->get_gameObject()->SetActive(false);

    modalUI->modal->set_name("BeatLeaderScoreDetailsModal");
    *modalUIPointer = modalUI;
}

void BeatLeader::ScoreDetailsPopup::updatePlayerDetails(Player player) {
    if (!PlayerController::IsIncognito(player)) {
        name->SetText(FormatUtils::FormatNameWithClans(player, 20, true), true);
        prestigeIcon->sprite = UIUtils::getPrestigeIcon(player);
        prestigeIcon->gameObject->SetActive(true);
        auto params = GetAvatarParams(player, false);
        playerAvatar->SetPlayer(player.avatar, params.baseMaterial, params.hueShift, params.saturation);
    } else {
        name->SetText("[REDACTED]", true);
        prestigeIcon->gameObject->SetActive(false);
        playerAvatar->SetHiddenPlayer();
    }
}

void BeatLeader::ScoreDetailsPopup::setScore(const Score& score) {
    scoreId = score.id;
    replayLink = score.replay;
    platform = score.platform;

    updatePlayerDetails(score.player);
    
    name->set_alignment(TMPro::TextAlignmentOptions::Center);
    rank->SetText(FormatUtils::FormatRank(score.player.rank, true), true);
    pp->SetText(FormatUtils::FormatPP(score.player.pp), true);

    if (score.player.profileSettings != nullopt) {
        sponsorMessage->SetText(score.player.profileSettings->message, true);
    } else {
        sponsorMessage->SetText("", true);
    }
    
    sponsorMessage->set_alignment(TMPro::TextAlignmentOptions::Center);

    playerButtons.setScore(score);
    general.setScore(score);

    scoreStatsFetched = false;

    selectTab(0);
}

void selectButton(BSML::ClickableImage* button, bool selected) {
    button->set_defaultColor(selected ? SelectedColor : FadedColor);
    button->set_highlightColor(selected ? SelectedColor : FadedHoverColor);
}

void BeatLeader::ScoreDetailsPopup::selectTab(int index) {
    selectButton(generalButton, false);
    selectButton(additionalButton, false);
    selectButton(overviewButton, false);
    selectButton(gridButton, false);
    selectButton(graphButton, false);
    loadingText->get_gameObject()->SetActive(false);

    general.setSelected(false);
    additional.setSelected(false);
    overview.setSelected(false);
    grid.setSelected(false);
    graph.setSelected(false);

    switch (index)
    {
    case 0:
        selectButton(generalButton, true);
        general.setSelected(true);
        break;
    case 1:
        selectButton(additionalButton, true);
        if (scoreStatsFetched) {
            additional.setScore(platform, scoreStats);
            additional.setSelected(true);
        }
        break;
    case 2:
        selectButton(overviewButton, true);
        if (scoreStatsFetched) {
            overview.setScore(scoreStats);
            overview.setSelected(true);
        }
        break;
    case 3:
        selectButton(gridButton, true);
        if (scoreStatsFetched) {
            grid.setScore(scoreStats);
            grid.setSelected(true);
        }
        break;
    case 4:
        selectButton(graphButton, true);
        if (scoreStatsFetched) {
            graph.setScore(scoreStats);
            graph.setSelected(true);
        }
        break;
    }

    if (index > 0 && !scoreStatsFetched) {
        auto self = this;
        self->loadingText->SetText("Loading...", true);
        loadingText->get_gameObject()->SetActive(true);
        string url = WebUtils::API_URL + "score/statistic/" + to_string(scoreId);
        WebUtils::GetJSONAsync(url, [self, index](long status, bool error, rapidjson::Document const& result) {
            if (status == 200 && !error && result.HasMember("scoreGraphTracker")) {
                scoreStats = ScoreStats(result);
                BSML::MainThreadScheduler::Schedule([self, index] {
                    self->scoreStatsFetched = true;
                    self->loadingText->get_gameObject()->SetActive(false);
                    self->selectTab(index);
                });
            } else {
                BSML::MainThreadScheduler::Schedule([self] {
                    self->loadingText->SetText("Failed to fetch stats", true);
                });

            }
        });
    }
}

void BeatLeader::ScoreDetailsPopup::setButtonsMaterial() const {
    if (ReplayInstalled()) {
        replayButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
        replayButton->set_defaultColor(FadedColor);
        replayButton->set_highlightColor(FadedHoverColor);
    }
    generalButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    additionalButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    overviewButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    gridButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    graphButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
}

void BeatLeader::ScoreDetailsPopup::playReplay() {
    string directory = getDataDir(modInfo) + "replays/temp/";
    filesystem::create_directories(directory);
    string file = directory + "replay.bsor";

    selectButton(generalButton, false);
    selectButton(overviewButton, false);
    selectButton(gridButton, false);
    selectButton(graphButton, false);

    loadingText->get_gameObject()->SetActive(true);
    general.setSelected(false);

    auto self = this;
    WebUtils::GetAsyncFile(replayLink, file, 64,
        [file, self](long httpCode) {
            if (httpCode == 200) {
                BSML::MainThreadScheduler::Schedule([file, self] {
                    if ((getModConfig().ShowReplaySettings.GetValue() && PlayReplayFromFile(file)) ||  (!getModConfig().ShowReplaySettings.GetValue() && PlayReplayFromFileWithoutSettings(file))) {
                        self->modal->Hide(true, nullptr);
                        ReplayManager::lastReplayFilename = file;
                    } else {
                        self->loadingText->SetText("Failed to parse the replay", true);
                    }
                });
            } else {
                BSML::MainThreadScheduler::Schedule([file, self] {
                    self->loadingText->SetText("Failed to download the replay", true);
                });
            }
    }, [self](float progress) {
        BSML::MainThreadScheduler::Schedule([progress, self] {
            self->loadingText->SetText("Downloading: " + to_string_wprecision(progress, 2) + "%", true);
        });
    });
    WebUtils::GetAsync(WebUtils::API_URL + "/watched/" + to_string(scoreId), [](long code, string result) {});
}