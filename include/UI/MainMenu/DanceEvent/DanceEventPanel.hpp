#pragma once

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Models/PlatformEvent.hpp"
#include "Models/DanceEventStatus.hpp"

#include "beatsaber-hook/shared/safeptr.hpp"

#include <functional>
#include <optional>

DECLARE_CLASS_CUSTOM(BeatLeader, DanceEventPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _background);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _headerBackground);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _header);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _fog);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _pumpkin);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _points);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _rank);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _maps);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _backButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _playlistButton);
};

namespace BeatLeader {

class DanceEventPanel : public ReeUIComponentV2<DanceEventPanelComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void SetEvent(PlatformEvent const& evt);
    void Refresh();

    std::function<void()> onBack;
    std::function<void()> onInitialized;

    static UnityEngine::RectTransform* Shown();
    static void SetShown(UnityEngine::RectTransform* content);

private:
    std::optional<PlatformEvent> event;
    safe_ptr<DanceEventPanelComponent*> safeComponent;
    int requestGeneration = 0;
    std::optional<DanceEventStatus> lastStatus;

    DanceEventPanelComponent* Usable();
    void ShowEvent();
    void SetMessage(std::string const& message);
    void SetData(DanceEventStatus const& status);
};

}
