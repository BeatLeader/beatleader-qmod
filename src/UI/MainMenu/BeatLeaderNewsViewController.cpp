#include "UI/MainMenu/BeatLeaderNewsViewController.hpp"
#include "bsml/shared/BSML.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/ReeUIComponentV2.hpp"

#include "UI/MainMenu/TextNews/TextNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/MapNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/EventNewsPanel.hpp"

DEFINE_TYPE(BeatLeader, BeatLeaderNewsViewController);

using namespace UnityEngine;

namespace BeatLeader {

    Vector2 BeatLeaderNewsViewController::GetTargetScreenSize() {
        return Vector2(286.0f, 80.0f);
    }

    void BeatLeaderNewsViewController::Awake() {
        auto transform = get_transform();
        textNewsPanel = TextNewsPanel::Instantiate<TextNewsPanel>(transform, false)->LocalComponent();
        mapNewsPanel = MapNewsPanel::Instantiate<MapNewsPanel>(transform, false)->LocalComponent();
        eventNewsPanel = EventNewsPanel::Instantiate<EventNewsPanel>(transform, false)->LocalComponent();
    }

    void BeatLeaderNewsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if (firstActivation) {
            BSML::parse_and_construct(get_content(), get_transform(), this);
        }
    }

    void BeatLeaderNewsViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
        get_gameObject()->SetActive(false);
        RevertScreenChanges();
    }

    void BeatLeaderNewsViewController::PostParse() {
        UpdateScreen();
    }

    void BeatLeaderNewsViewController::OnEnable() {
        if (!initialized) return;
        UpdateScreen();
    }

    void BeatLeaderNewsViewController::OnDisable() {
        RevertScreenChanges();
    }

    bool BeatLeaderNewsViewController::LazyInitializeScreen() {
        if (initialized) return true;
        if (!screen) return false;
        
        screenTransform = screen->GetComponent<RectTransform*>();
        originalScreenSize = screenTransform->get_sizeDelta();
        originalPosition = screenTransform->get_anchoredPosition();
        initialized = true;
        return true;
    }

    void BeatLeaderNewsViewController::UpdateScreen() {
        if (!LazyInitializeScreen()) return;
        screenTransform->set_sizeDelta(GetTargetScreenSize());
        screenTransform->set_anchoredPosition(Vector2(originalPosition.x - 20.0f, originalPosition.y));
    }

    void BeatLeaderNewsViewController::RevertScreenChanges() {
        if (!LazyInitializeScreen()) return;
        screenTransform->set_sizeDelta(originalScreenSize);
        screenTransform->set_anchoredPosition(originalPosition);
    }
} 