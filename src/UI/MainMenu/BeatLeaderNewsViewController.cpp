#include "UI/MainMenu/BeatLeaderNewsViewController.hpp"
#include "bsml/shared/BSML.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/ReeUIComponentV2.hpp"

#include "UI/MainMenu/TextNews/TextNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/MapNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/EventNewsPanel.hpp"
#include "UI/MainMenu/DanceEvent/DanceEventPanel.hpp"
#include "Core/Dance/DanceEvent.hpp"
#include "Utils/ReplayManager.hpp"

DEFINE_TYPE(BeatLeader, BeatLeaderNewsViewController);

using namespace UnityEngine;

namespace BeatLeader {

    static safe_ptr<BeatLeaderNewsViewController*> instance;
    static bool danceAutoOpened = false;

    static void HandleDanceEventChanged() {
        if (instance) instance->CheckDanceEvent();
    }

    static void HandlePlayUploadChanged() {
        if (instance && ReplayManager::LastPlayUpload().done) instance->RefreshDanceEvent();
    }

    static DanceEventPanel* NativePanel(DanceEventPanelComponent* component) {
        return reinterpret_cast<DanceEventPanel*>(component->nativeComponent);
    }

    static bool IsReady(ReeComponent* component) {
        return component->_state == ReeComponentState::HierarchySet;
    }

    Vector2 BeatLeaderNewsViewController::GetTargetScreenSize() {
        return Vector2(286.0f, 80.0f);
    }

    void BeatLeaderNewsViewController::Awake() {
        auto transform = get_transform();
        textNewsPanel = TextNewsPanel::Instantiate<TextNewsPanel>(transform, false)->LocalComponent();
        mapNewsPanel = MapNewsPanel::Instantiate<MapNewsPanel>(transform, false)->LocalComponent();
        eventNewsPanel = EventNewsPanel::Instantiate<EventNewsPanel>(transform, false)->LocalComponent();

        auto dancePanel = DanceEventPanel::Instantiate<DanceEventPanel>(transform, false);
        danceEventPanel = dancePanel->LocalComponent();
        dancePanel->onBack = [] {
            if (instance) instance->HideDanceEvent();
        };
        dancePanel->onInitialized = [] {
            if (instance) instance->ApplyDancePresentation();
        };

        instance = this;
        static bool listening = false;
        if (!listening) {
            listening = true;
            DanceEvent::AddCurrentChangedListener(HandleDanceEventChanged);
            ReplayManager::AddPlayUploadListener(HandlePlayUploadChanged);
        }
    }

    void BeatLeaderNewsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if (firstActivation) {
            BSML::parse_and_construct(get_content(), get_transform(), this);
            ApplyDancePresentation();
            CheckDanceEvent();
        } else {
            RefreshDanceEvent();
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

    #pragma region Dance event

    void BeatLeaderNewsViewController::CheckDanceEvent() {
        auto const& current = DanceEvent::Current();
        if (!current) {
            if (dancePresented) HideDanceEvent();
            return;
        }
        NativePanel(danceEventPanel)->SetEvent(*current);
        if (!danceAutoOpened) {
            danceAutoOpened = true;
            PresentDanceEvent();
        }
    }

    void BeatLeaderNewsViewController::PresentDanceEvent() {
        auto const& current = DanceEvent::Current();
        if (!current) return;
        dancePresented = true;
        auto panel = NativePanel(danceEventPanel);
        panel->SetEvent(*current);
        panel->Refresh();
        ApplyDancePresentation();
    }

    void BeatLeaderNewsViewController::HideDanceEvent() {
        dancePresented = false;
        ApplyDancePresentation();
    }

    void BeatLeaderNewsViewController::RefreshDanceEvent() {
        if (dancePresented && DanceEvent::Current()) NativePanel(danceEventPanel)->Refresh();
    }

    void BeatLeaderNewsViewController::ApplyDancePresentation() {
        if (IsReady(eventNewsPanel)) eventNewsPanel->SetRootActive(!dancePresented);
        if (IsReady(mapNewsPanel)) mapNewsPanel->SetRootActive(!dancePresented);
        if (IsReady(danceEventPanel)) {
            danceEventPanel->SetRootActive(dancePresented);
            DanceEventPanel::SetShown(dancePresented ? danceEventPanel->_content->GetComponent<RectTransform*>() : nullptr);
        }
    }

    void BeatLeaderNewsViewController::PresentDanceEventPanel() {
        if (instance) instance->PresentDanceEvent();
    }

    #pragma endregion
}
