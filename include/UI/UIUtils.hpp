#pragma once

#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

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