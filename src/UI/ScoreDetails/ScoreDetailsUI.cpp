#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/UI/EmojiSupport.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
static UnityEngine::Color FadedColor = UnityEngine::Color(0.8f, 0.8f, 0.8f, 0.2f);
static UnityEngine::Color FadedHoverColor = UnityEngine::Color(0.5f, 0.5f, 0.5f, 0.2f);

void BeatLeader::initScoreDetailsPopup(BeatLeader::ScoreDetailsPopup** modalUIPointer, Transform* parent){
    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::ScoreDetailsPopup*) malloc(sizeof(BeatLeader::ScoreDetailsPopup));
    UnityEngine::Sprite* roundRect = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Sprite*>().FirstOrDefault([](UnityEngine::Sprite* x) { return x->get_name() == "RoundRect10"; });
    modalUI->modal = CreateModal(parent, UnityEngine::Vector2(60, 47), [](HMUI::ModalView *modal) {}, true);

    auto playerAvatarImage = ::QuestUI::BeatSaberUI::CreateImage(modalUI->modal->get_transform(), NULL, UnityEngine::Vector2(0, 30), UnityEngine::Vector2(24, 24));
    modalUI->playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
    modalUI->playerAvatar->Init(playerAvatarImage);

    modalUI->rank = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(6.0, 16.0));
    
    modalUI->name = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(0.0, 18.0));
    modalUI->name->get_gameObject()->AddComponent<::QuestUI::Backgroundable*>()->ApplyBackground("round-rect-panel");

    modalUI->sponsorMessage = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(0, -28));

    EmojiSupport::AddSupport(modalUI->name);

    modalUI->pp = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(45.0, 16.0));

    modalUI->general = GeneralScoreDetails(modalUI->modal);
    modalUI->overview = ScoreStatsOverview(modalUI->modal);
    modalUI->grid = ScoreStatsGrid(modalUI->modal);
    modalUI->graph = ScoreStatsGraph(modalUI->modal);

    modalUI->generalButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalUI->modal->get_transform(), BundleLoader::bundle->overviewIcon, UnityEngine::Vector2(-10.5, -20), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(0);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->generalButton, "General score info");
    
    modalUI->overviewButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalUI->modal->get_transform(), BundleLoader::bundle->detailsIcon, UnityEngine::Vector2(-3.5, -20), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(1);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->overviewButton, "Detailed score info");

    modalUI->gridButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalUI->modal->get_transform(), BundleLoader::bundle->gridIcon, UnityEngine::Vector2(3.5, -20), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(2);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->gridButton, "Note accuracy distribution");

    modalUI->graphButton = ::QuestUI::BeatSaberUI::CreateClickableImage(modalUI->modal->get_transform(), BundleLoader::bundle->graphIcon, UnityEngine::Vector2(10.5, -20), UnityEngine::Vector2(5, 5), [modalUI](){
        modalUI->selectTab(3);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->graphButton, "Accuracy timeline graph");

    modalUI->setButtonsMaterial();

    modalUI->loadingText = CreateText(modalUI->modal->get_transform(), "Loading...", UnityEngine::Vector2(0.0, 0.0));
    modalUI->loadingText->set_alignment(TMPro::TextAlignmentOptions::Center);
    modalUI->loadingText->get_gameObject()->SetActive(false);

    modalUI->modal->set_name("BLScoreDetailsModal");
    *modalUIPointer = modalUI;
}

void BeatLeader::ScoreDetailsPopup::setScore(const Score& score) {
    scoreId = score.id;

    playerAvatar->SetPlayer(score.player.avatar, score.player.role);

    name->SetText(FormatUtils::FormatNameWithClans(score.player, 20));
    name->set_alignment(TMPro::TextAlignmentOptions::Center);
    rank->SetText(FormatUtils::FormatRank(score.player.rank, true));
    pp->SetText(FormatUtils::FormatPP(score.player.pp));

    sponsorMessage->SetText(score.player.sponsorMessage);
    sponsorMessage->set_alignment(TMPro::TextAlignmentOptions::Center);

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
            if (status == 200) {
                self->scoreStats = ScoreStats(result);
                self->scoreStatsFetched = true;
                QuestUI::MainThreadScheduler::Schedule([self, index] {
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

void BeatLeader::ScoreDetailsPopup::setButtonsMaterial() {
    generalButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    overviewButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    gridButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
    graphButton->set_material(BundleLoader::bundle->UIAdditiveGlowMaterial);
}