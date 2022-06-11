#include "include/UI/VotingUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/UI/EmojiSupport.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/UI/ColorBlock.hpp"
#include "UnityEngine/UI/Selectable_SelectionState.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

void setupTypeButton(UnityEngine::UI::Button* button) {
    auto title = button->get_transform()->GetComponentsInChildren<TMPro::TextMeshProUGUI*>()->get(0);
    title->set_fontSize(3);
}

void moveTitelButton(UnityEngine::UI::Button* button) {
    auto title = button->get_transform()->GetComponentsInChildren<TMPro::TextMeshProUGUI*>()->get(0);
    title->set_alignment(TMPro::TextAlignmentOptions::Right);
}

void setButtonTitleColor(UnityEngine::UI::Button* button, UnityEngine::Color32 color) {
    auto title = button->get_transform()->GetComponentsInChildren<TMPro::TextMeshProUGUI*>()->get(0);
    title->SetFaceColor(color);
}

void BeatLeader::initVotingPopup(
    BeatLeader::RankVotingPopup** modalUIPointer, 
    Transform* parent,
    function<void(bool, bool, float, int)> const &callback){
    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::RankVotingPopup*) malloc(sizeof(BeatLeader::RankVotingPopup));
    UnityEngine::Sprite* roundRect = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Sprite*>().FirstOrDefault([](UnityEngine::Sprite* x) { return x->get_name() == "RoundRect10"; });

    auto container = CreateModal(parent, UnityEngine::Vector2(60, 30), [](HMUI::ModalView *modal) {}, true);
    modalUI->modal = container;

    // Page 1
    modalUI->header1 = CreateText(container->get_transform(), "Is this map suitable for rank?", UnityEngine::Vector2(4.0, 8.0));
    modalUI->yesButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "YES", UnityEngine::Vector2(-12.0, 3.0), [modalUI]() {
        modalUI->rankable = true;
        modalUI->rightButton->get_gameObject()->SetActive(true);
        modalUI->voteButton->set_interactable(true);

        setButtonTitleColor(modalUI->noButton, UnityEngine::Color32(255, 255, 255, 255));
        setButtonTitleColor(modalUI->yesButton, UnityEngine::Color32(102, 255, 102, 255));
    });

    modalUI->noButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "NO", UnityEngine::Vector2(14.0, 3.0), [modalUI]() {
        modalUI->rankable = false;
        modalUI->rightButton->get_gameObject()->SetActive(false);
        modalUI->voteButton->set_interactable(true);

        setButtonTitleColor(modalUI->noButton, UnityEngine::Color32(255, 102, 102, 255));
        setButtonTitleColor(modalUI->yesButton, UnityEngine::Color32(255, 255, 255, 255));
    });

    // Page 2
    modalUI->header2 = CreateText(container->get_transform(), "Difficulty and category (optional)", UnityEngine::Vector2(4.0, 8.0));
    modalUI->starSlider = CreateSliderSetting(
        modalUI->modal->get_transform(),
        "Stars",
        0.1,
        0,
        1,
        15,
        UnityEngine::Vector2(4, -7),
        [modalUI](float stars) {
       modalUI->stars = stars; 
    });
    move(modalUI->starSlider->slider, -7, 0);

    modalUI->accButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "acc", UnityEngine::Vector2(-20, -3), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::acc, modalUI->accButton);
    });
    setupTypeButton(modalUI->accButton);

    modalUI->techButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "tech", UnityEngine::Vector2(-6, -3), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::tech, modalUI->techButton);
    });
    setupTypeButton(modalUI->techButton);

    modalUI->midspeedButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "midspeed", UnityEngine::Vector2(8, -3), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::midspeed, modalUI->midspeedButton);
    });
    setupTypeButton(modalUI->midspeedButton);

    modalUI->speedButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "speed", UnityEngine::Vector2(22, -3), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::speed, modalUI->speedButton);
    });
    setupTypeButton(modalUI->speedButton);

    modalUI->voteButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Vote", UnityEngine::Vector2(-12.0, -10.0), UnityEngine::Vector2(15.0, 8.0), [callback, modalUI]() {
        callback(true, modalUI->rankable, modalUI->stars, modalUI->type);
    });
    moveTitelButton(modalUI->voteButton);

    modalUI->cancelButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Cancel", UnityEngine::Vector2(12.0, -10.0), UnityEngine::Vector2(15.0, 8.0), [callback]() {
        callback(false, false, 0, 0);
    });
    moveTitelButton(modalUI->cancelButton);

    modalUI->leftButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "<", UnityEngine::Vector2(-27.0, -10.0), UnityEngine::Vector2(5.0, 6.0), [modalUI]() {
        modalUI->left();
    });
    setupTypeButton(modalUI->leftButton);

    modalUI->rightButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), ">", UnityEngine::Vector2(26.0, -10.0), UnityEngine::Vector2(5.0, 6.0), [modalUI]() {
        modalUI->right();
    });
    setupTypeButton(modalUI->rightButton);

    modalUI->modal->set_name("BLVotingModal");
    *modalUIPointer = modalUI;
}

void BeatLeader::RankVotingPopup::reset() {
    header1->get_gameObject()->SetActive(true);
    header2->get_gameObject()->SetActive(false);

    accButton->get_gameObject()->SetActive(false);
    techButton->get_gameObject()->SetActive(false);
    midspeedButton->get_gameObject()->SetActive(false);
    speedButton->get_gameObject()->SetActive(false);
    
    setButtonTitleColor(accButton, UnityEngine::Color32(255, 255, 255, 255));
    setButtonTitleColor(techButton, UnityEngine::Color32(255, 255, 255, 255));
    setButtonTitleColor(midspeedButton, UnityEngine::Color32(255, 255, 255, 255));
    setButtonTitleColor(speedButton, UnityEngine::Color32(255, 255, 255, 255));

    setButtonTitleColor(noButton, UnityEngine::Color32(255, 255, 255, 255));
    setButtonTitleColor(yesButton, UnityEngine::Color32(255, 255, 255, 255));

    leftButton->get_gameObject()->SetActive(false);
    rightButton->get_gameObject()->SetActive(false);

    yesButton->get_gameObject()->SetActive(true);
    noButton->get_gameObject()->SetActive(true);

    starSlider->get_gameObject()->SetActive(false);

    voteButton->set_interactable(false);

    type = 0;
    stars = 0;
}

void BeatLeader::RankVotingPopup::left() {
    rightButton->get_gameObject()->SetActive(true);
    leftButton->get_gameObject()->SetActive(false);

    accButton->get_gameObject()->SetActive(false);
    techButton->get_gameObject()->SetActive(false);
    midspeedButton->get_gameObject()->SetActive(false);
    speedButton->get_gameObject()->SetActive(false);

    yesButton->get_gameObject()->SetActive(true);
    noButton->get_gameObject()->SetActive(true);

    starSlider->get_gameObject()->SetActive(false);

    header1->get_gameObject()->SetActive(true);
    header2->get_gameObject()->SetActive(false);
}

void BeatLeader::RankVotingPopup::right() {
    leftButton->get_gameObject()->SetActive(true);
    rightButton->get_gameObject()->SetActive(false);

    accButton->get_gameObject()->SetActive(true);
    techButton->get_gameObject()->SetActive(true);
    midspeedButton->get_gameObject()->SetActive(true);
    speedButton->get_gameObject()->SetActive(true);

    yesButton->get_gameObject()->SetActive(false);
    noButton->get_gameObject()->SetActive(false);

    starSlider->get_gameObject()->SetActive(true);

    header1->get_gameObject()->SetActive(false);
    header2->get_gameObject()->SetActive(true);
}

void BeatLeader::RankVotingPopup::updateType(MapType mapType, UnityEngine::UI::Button* button) {
    
    if ((type & mapType) != 0) {
        type &= ~mapType;
        setButtonTitleColor(button, UnityEngine::Color32(255, 255, 255, 255));
    } else {
        type |= mapType;
        setButtonTitleColor(button, UnityEngine::Color32(153, 255, 255, 255));
    }
}