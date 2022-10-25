#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/Models/ScoreStats.hpp"

#include "include/UI/EmojiSupport.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/UI/Themes/ThemeUtils.hpp"

#include "include/Core/ReplayPlayer.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "main.hpp"

#include <sstream>
#include <filesystem>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
static UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);
static UnityEngine::Color FadedHoverColor = UnityEngine::Color(0.5f, 0.5f, 0.5f, 0.2f);

static string replayLink;
static ScoreStats scoreStats;

static void MakeModalTransparent(HMUI::ModalView *modal) {
    int childCount = modal->get_transform()->get_childCount();
    for (int i = 0; i < childCount; i++) {
        auto* child = modal->get_transform()->GetChild(i)->GetComponent<RectTransform*>();

        if (child->get_gameObject()->get_name() == "BG") {
            child->GetComponent<Image*>()->set_sprite(Sprites::get_TransparentPixel());
        }
    }
}

void BeatLeader::initScoreDetailsPopup(
        BeatLeader::ScoreDetailsPopup** modalUIPointer, 
        Transform* parent,
        function<void()> const &incognitoCallback){
    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::ScoreDetailsPopup*) malloc(sizeof(BeatLeader::ScoreDetailsPopup));
    modalUI->modal = CreateModal(parent, UnityEngine::Vector2(60, 90), [](HMUI::ModalView *modal) {}, true);
    MakeModalTransparent(modalUI->modal);

    auto modalTransform = modalUI->modal->get_transform();

    auto playerAvatarImage = ::QuestUI::BeatSaberUI::CreateImage(modalTransform, NULL, UnityEngine::Vector2(0, 30), UnityEngine::Vector2(24, 24));
    modalUI->playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
    modalUI->playerAvatar->Init(playerAvatarImage);

    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, 18), UnityEngine::Vector2(58, 6));
    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, -3), UnityEngine::Vector2(60, 34));
    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0, -24), UnityEngine::Vector2(30, 6));
    UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(-24.5, -24), UnityEngine::Vector2(7, 7));

    modalUI->rank = CreateText(modalTransform, "", UnityEngine::Vector2(6.0, 16.0));
    modalUI->name = CreateText(modalTransform, "", UnityEngine::Vector2(0.0, 18.0));
    modalUI->sponsorMessage = CreateText(modalTransform, "", UnityEngine::Vector2(0, -32));

    EmojiSupport::AddSupport(modalUI->name);

    modalUI->pp = CreateText(modalTransform, "", UnityEngine::Vector2(45.0, 16.0));

    modalUI->playerButtons.Setup(modalUI->modal, [modalUI, incognitoCallback](Player player) {
        modalUI->updatePlayerDetails(player);
        incognitoCallback();
    });
    modalUI->general = GeneralScoreDetails(modalUI->modal);
    modalUI->overview = ScoreStatsOverview(modalUI->modal);
    modalUI->grid = ScoreStatsGrid(modalUI->modal);
    modalUI->graph = ScoreStatsGraph(modalUI->modal);

    modalUI->generalButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->overview1Icon, UnityEngine::Vector2(-10.5, -24), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(0);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->generalButton, "General score info");

    modalUI->overviewButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->detailsIcon, UnityEngine::Vector2(-3.5, -24), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(1);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->overviewButton, "Detailed score info");

    modalUI->gridButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->gridIcon, UnityEngine::Vector2(3.5, -24), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(2);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->gridButton, "Note accuracy distribution");

    modalUI->graphButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->graphIcon, UnityEngine::Vector2(10.5, -24), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(3);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->graphButton, "Accuracy timeline graph");

    if (ReplayInstalled()) {
        modalUI->replayButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->replayIcon, UnityEngine::Vector2(-24.5, -24), UnityEngine::Vector2(5, 5), [modalUI](){
            modalUI->playReplay();
        });
        ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->replayButton, "Watch the replay");
    }

    modalUI->setButtonsMaterial();

    modalUI->loadingText = CreateText(modalTransform, "Loading...", UnityEngine::Vector2(0.0, 0.0));
    modalUI->loadingText->set_alignment(TMPro::TextAlignmentOptions::Center);
    modalUI->loadingText->get_gameObject()->SetActive(false);

    modalUI->modal->set_name("BLScoreDetailsModal");
    *modalUIPointer = modalUI;
}

void BeatLeader::ScoreDetailsPopup::updatePlayerDetails(Player player) {
    if (!PlayerController::IsIncognito(player)) {
        name->SetText(FormatUtils::FormatNameWithClans(player, 20));
        auto params = GetAvatarParams(player, false);
        playerAvatar->SetPlayer(player.avatar, params.baseMaterial, params.hueShift, params.saturation);
    } else {
        name->SetText("[REDACTED]");
        playerAvatar->SetHiddenPlayer();
    }
}

void BeatLeader::ScoreDetailsPopup::setScore(const Score& score) {
    scoreId = score.id;
    replayLink = score.replay;

    updatePlayerDetails(score.player);
    
    name->set_alignment(TMPro::TextAlignmentOptions::Center);
    rank->SetText(FormatUtils::FormatRank(score.player.rank, true));
    pp->SetText(FormatUtils::FormatPP(score.player.pp));

    if (score.player.profileSettings != nullopt) {
        sponsorMessage->SetText(score.player.profileSettings->message);
    } else {
        sponsorMessage->SetText("");
    }
    
    sponsorMessage->set_alignment(TMPro::TextAlignmentOptions::Center);

    playerButtons.setScore(score);
    general.setScore(score);

    scoreStatsFetched = false;

    selectTab(0);
}

void selectButton(QuestUI::ClickableImage* button, bool selected) {
    button->set_defaultColor(selected ? SelectedColor : FadedColor);
    button->set_highlightColor(selected ? SelectedColor : FadedHoverColor);
}

void BeatLeader::ScoreDetailsPopup::selectTab(int index) {
    selectButton(generalButton, false);
    selectButton(overviewButton, false);
    selectButton(gridButton, false);
    selectButton(graphButton, false);
    loadingText->get_gameObject()->SetActive(false);

    general.setSelected(false);
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
        selectButton(overviewButton, true);
        if (scoreStatsFetched) {
            overview.setScore(scoreStats);
            overview.setSelected(true);
        }
        break;
    case 2:
        selectButton(gridButton, true);
        if (scoreStatsFetched) {
            grid.setScore(scoreStats);
            grid.setSelected(true);
        }
        break;
    case 3:
        selectButton(graphButton, true);
        if (scoreStatsFetched) {
            graph.setScore(scoreStats);
            graph.setSelected(true);
        }
        break;
    }

    if (index > 0 && !scoreStatsFetched) {
        auto self = this;
        self->loadingText->SetText("Loading...");
        loadingText->get_gameObject()->SetActive(true);
        string url = WebUtils::API_URL + "score/statistic/" + to_string(scoreId);
        WebUtils::GetJSONAsync(url, [self, index](long status, bool error, rapidjson::Document const& result) {
            if (status == 200 && !error && result.HasMember("scoreGraphTracker")) {
                scoreStats = ScoreStats(result);
                QuestUI::MainThreadScheduler::Schedule([self, index] {
                    self->scoreStatsFetched = true;
                    self->loadingText->get_gameObject()->SetActive(false);
                    self->selectTab(index);
                });
            } else {
                QuestUI::MainThreadScheduler::Schedule([self] {
                    self->loadingText->SetText("Failed to fetch stats");
                });

            }
        });
    }
}

void BeatLeader::ScoreDetailsPopup::setButtonsMaterial() const {
    if (replayButton) {
        replayButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
        replayButton->set_defaultColor(FadedColor);
        replayButton->set_highlightColor(FadedHoverColor);
    }
    generalButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
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
                QuestUI::MainThreadScheduler::Schedule([file, self] {
                    if (PlayReplayFromFile(file)) {
                        self->modal->Hide(true, nullptr);
                    } else {
                        self->loadingText->SetText("Failed to parse the replay");
                    }
                });
            } else {
                QuestUI::MainThreadScheduler::Schedule([file, self] {
                    self->loadingText->SetText("Failed to download the replay");
                });
            }
    }, [self](float progress) {
        QuestUI::MainThreadScheduler::Schedule([progress, self] {
            self->loadingText->SetText("Downloading: " + to_string_wprecision(progress, 2) + "%");
        });
    });
}