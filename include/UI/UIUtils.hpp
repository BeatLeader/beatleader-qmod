#pragma once

#include "include/Models/TriangleRating.hpp"

#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"

#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/Screen.hpp"
#include "HMUI/ViewController.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/ArrayUtil.hpp"
#include "questui/shared/BeatSaberUI.hpp"

inline void move(UnityEngine::Component* label, float x, float y) {
    UnityEngine::RectTransform* transform = label->GetComponent<UnityEngine::RectTransform *>();
    UnityEngine::Vector2 position = transform->get_anchoredPosition();
    position.x += x;
    position.y += y;
    transform->set_anchoredPosition(position);
}

inline void resize(UnityEngine::Component* label, float x, float y) {
    UnityEngine::RectTransform* transform = label->GetComponent<UnityEngine::RectTransform *>();
    UnityEngine::Vector2 sizeDelta = transform->get_sizeDelta();
    sizeDelta.x += x;
    sizeDelta.y += y;
    transform->set_sizeDelta(sizeDelta);
}

inline UnityEngine::GameObject* CreateCustomScreen(HMUI::ViewController* rootView, UnityEngine::Vector2 screenSize, UnityEngine::Vector3 position, float curvatureRadius) {
    auto gameObject = QuestUI::BeatSaberUI::CreateCanvas();
    auto screen = gameObject->AddComponent<HMUI::Screen*>();
    screen->rootViewController = rootView;

    auto transform = gameObject->get_transform();
    UnityEngine::GameObject* screenSystem = UnityEngine::GameObject::Find("ScreenContainer");
    if(screenSystem) {
        transform->set_position(screenSystem->get_transform()->get_position());
        screen->get_gameObject()->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta(screenSize);
    }
    return gameObject;
}

inline void EnableHorizontalFit(UnityEngine::UI::HorizontalLayoutGroup* layoutGroup) {
    layoutGroup->set_childForceExpandHeight(false);
    layoutGroup->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
    auto groupFitter = layoutGroup->GetComponent<UnityEngine::UI::ContentSizeFitter*>();
    groupFitter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
}

namespace UIUtils {
    extern SafePtrUnity<HMUI::ImageView> roundRectSprite;

    HMUI::ImageView* getRoundRectSprite();
    HMUI::ImageView* GetCopyOf(HMUI::ImageView* comp, HMUI::ImageView* other);
    void OpenSettings();
    HMUI::ImageView* CreateRoundRectImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta);
    float getStarsToShow(TriangleRating rating);
}