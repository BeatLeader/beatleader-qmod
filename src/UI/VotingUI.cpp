#include "include/UI/VotingUI.hpp"
#include "UnityEngine/zzzz__Vector2_def.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
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

#include "bsml/shared/BSML/Components/Backgroundable.hpp"

#include "main.hpp"

#include "UI/LinksContainer.hpp"

#include <sstream>

using namespace BSML::Lite;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

void setupButtonTitle(UnityEngine::UI::Button* button, float offset, int fontSize = 0) {
    UnityEngine::Object::Destroy(button->get_transform()->Find("Content")->GetComponent<UnityEngine::UI::LayoutElement*>());

    auto title = button->GetComponentsInChildren<TMPro::TextMeshProUGUI*>()[0];
    if (fontSize > 0) {
        title->set_fontSize(fontSize);
    }
    
    title->set_margin(UnityEngine::Vector4(offset,offset,offset,offset));
}

void setButtonTitleColor(UnityEngine::UI::Button* button, UnityEngine::Color32 const& color) {
    auto title = button->GetComponentsInChildren<TMPro::TextMeshProUGUI*>()[0];
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

    auto container = CreateModal(parent, UnityEngine::Vector2(60, 30), nullptr, true);
    modalUI->modal = container;

    auto containerTransform = container->get_transform();

    // Page 1
    modalUI->header1 = CreateText(containerTransform, "Is this map suitable for rank?", UnityEngine::Vector2(-16.0, 10));
    modalUI->noButton = ::BSML::Lite::CreateUIButton(containerTransform, "NO", UnityEngine::Vector2(18.0, -13.0), [modalUI]() {
        modalUI->rankable = false;
        modalUI->rightButton->get_gameObject()->SetActive(false);
        modalUI->voteButton->set_interactable(true);

        setButtonTitleColor(modalUI->noButton, UnityEngine::Color32(0, 255, 102, 102, 255));
        setButtonTitleColor(modalUI->yesButton, UnityEngine::Color32(0, 255, 255, 255, 255));
    });

    BeatLeader::SetButtonSize(modalUI->noButton, UnityEngine::Vector2(20.0, 10.0));
    SetButtonTextSize(modalUI->noButton, 6.0f);

    modalUI->yesButton = ::BSML::Lite::CreateUIButton(containerTransform, "YES", UnityEngine::Vector2(42.0, -13.0), [modalUI]() {
        modalUI->rankable = true;
        modalUI->rightButton->get_gameObject()->SetActive(true);
        modalUI->voteButton->set_interactable(true);

        setButtonTitleColor(modalUI->noButton, UnityEngine::Color32(0, 255, 255, 255, 255));
        setButtonTitleColor(modalUI->yesButton, UnityEngine::Color32(0, 102, 255, 102, 255));
    });

    BeatLeader::SetButtonSize(modalUI->yesButton, UnityEngine::Vector2(20.0, 10.0));
    SetButtonTextSize(modalUI->yesButton, 6.0f);

    // Page 2
    modalUI->header2 = CreateText(containerTransform, "<u>Difficulty and category (optional)", UnityEngine::Vector2(-19.0, 14.0));
    modalUI->header2->color = UnityEngine::Color(0.667f, 0.667f, 0.667f, 1.0f);

    modalUI->starSlider = CreateSliderSetting(
        modalUI->modal->get_transform(),
        "",
        0.1f,
        0.0f,
        0,
        15,
        0.0f,
        true,
        UnityEngine::Vector2(4, -10),
        [modalUI](float stars) {
       modalUI->stars = stars; 
    });
    modalUI->starSlider->formatter = [](float value) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << value << "*";
        return value > 0.0f ? ss.str() : "Skip";
    };
    resize(modalUI->starSlider->slider, 4.5, 0);
    move(modalUI->starSlider->slider, -4.5, 0);

    float typeButtonY = -17.0;

    modalUI->accButton = ::BSML::Lite::CreateUIButton(containerTransform, "acc", UnityEngine::Vector2(9, typeButtonY), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::acc, modalUI->accButton);
    });
    BeatLeader::SetButtonSize(modalUI->accButton, UnityEngine::Vector2(12.0, 7.0));
    setupButtonTitle(modalUI->accButton, -0.5, 3);

    modalUI->techButton = ::BSML::Lite::CreateUIButton(containerTransform, "tech", UnityEngine::Vector2(23, typeButtonY), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::tech, modalUI->techButton);
    });
    BeatLeader::SetButtonSize(modalUI->techButton, UnityEngine::Vector2(12.0, 7.0));
    setupButtonTitle(modalUI->techButton, -0.5, 3);

    modalUI->midspeedButton = ::BSML::Lite::CreateUIButton(containerTransform, "midspeed", UnityEngine::Vector2(37, typeButtonY), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::midspeed, modalUI->midspeedButton);
    });
    BeatLeader::SetButtonSize(modalUI->midspeedButton, UnityEngine::Vector2(12.0, 7.0));
    setupButtonTitle(modalUI->midspeedButton, -0.5, 3);

    modalUI->speedButton = ::BSML::Lite::CreateUIButton(containerTransform, "speed", UnityEngine::Vector2(51, typeButtonY), UnityEngine::Vector2(12.0, 6.0), [modalUI]() {
        modalUI->updateType(MapType::speed, modalUI->speedButton);
    });
    BeatLeader::SetButtonSize(modalUI->speedButton, UnityEngine::Vector2(12.0, 7.0));
    setupButtonTitle(modalUI->speedButton, -0.5, 3);

    modalUI->voteButton = ::BSML::Lite::CreateUIButton(containerTransform, "Submit", UnityEngine::Vector2(30, -25.0), UnityEngine::Vector2(16.0, 3.0), [callback, modalUI]() {
        callback(true, modalUI->rankable, modalUI->stars, modalUI->type);
    });
    BeatLeader::SetButtonSize(modalUI->voteButton, UnityEngine::Vector2(16.0, 8.0));
    setupButtonTitle(modalUI->voteButton, -0.5);

    modalUI->leftButton = ::BSML::Lite::CreateUIButton(containerTransform, "◄", UnityEngine::Vector2(10, -25.0), UnityEngine::Vector2(5.0, 6.0), [modalUI]() {
        modalUI->left();
    });
    BeatLeader::SetButtonSize(modalUI->leftButton, UnityEngine::Vector2(11.0, 8.0));

    modalUI->rightButton = ::BSML::Lite::CreateUIButton(containerTransform, "►", UnityEngine::Vector2(10, -25.0), UnityEngine::Vector2(5.0, 6.0), [modalUI]() {
        modalUI->right();
    });
    BeatLeader::SetButtonSize(modalUI->rightButton, UnityEngine::Vector2(11.0, 8.0));

    modalUI->modal->set_name("BeatLeaderVotingModal");
    *modalUIPointer = modalUI;
}

void BeatLeader::RankVotingPopup::reset() {
    header1->get_gameObject()->SetActive(true);
    header2->get_gameObject()->SetActive(false);

    accButton->get_gameObject()->SetActive(false);
    techButton->get_gameObject()->SetActive(false);
    midspeedButton->get_gameObject()->SetActive(false);
    speedButton->get_gameObject()->SetActive(false);
    
    setButtonTitleColor(accButton, UnityEngine::Color32(0, 255, 255, 255, 255));
    setButtonTitleColor(techButton, UnityEngine::Color32(0, 255, 255, 255, 255));
    setButtonTitleColor(midspeedButton, UnityEngine::Color32(0, 255, 255, 255, 255));
    setButtonTitleColor(speedButton, UnityEngine::Color32(0, 255, 255, 255, 255));

    setButtonTitleColor(noButton, UnityEngine::Color32(0, 255, 255, 255, 255));
    setButtonTitleColor(yesButton, UnityEngine::Color32(0, 255, 255, 255, 255));

    leftButton->get_gameObject()->SetActive(false);
    rightButton->get_gameObject()->SetActive(false);

    yesButton->get_gameObject()->SetActive(true);
    noButton->get_gameObject()->SetActive(true);

    starSlider->get_gameObject()->SetActive(false);
    starSlider->set_Value(0);

    voteButton->set_interactable(false);

    type = 0;
    stars = 0;
}

void BeatLeader::RankVotingPopup::left() const {
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

void BeatLeader::RankVotingPopup::right() const {
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
        setButtonTitleColor(button, UnityEngine::Color32(0, 255, 255, 255, 255));
    } else {
        type |= mapType;
        setButtonTitleColor(button, UnityEngine::Color32(0, 153, 255, 255, 255));
    }
}