#pragma once

#include "HMUI/ModalView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "Models/PlatformEvent.hpp"
#include "UI/Abstract/AbstractReeModal.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, EventDetailsDialogComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _eventContainer);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _eventDescription);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _downloadButton);
};

namespace BeatLeader {

class EventDetailsDialog : public AbstractReeModal<EventDetailsDialogComponent*> {
public:
    using ComponentType = EventDetailsDialogComponent*;
    
    bool downloadButtonActive;
    PlatformEvent* context;

    void OnContextChanged() override;
    void OnResume() override;
    void SetContext(Il2CppObject* context) override {
        this->context = reinterpret_cast<PlatformEvent*>(context);
        OnContextChanged();
    }
    void HandleDownloadButtonClicked();
    
    // Override from AbstractReeModal
    void OnInitialize() override;
    StringW GetContent() override;
};

} 