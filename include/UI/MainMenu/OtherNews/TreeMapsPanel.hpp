#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/ImageView.hpp"
#include "Models/TreeStatus.hpp"
#include "API/RequestManager.hpp"

#include "UI/MainMenu/OtherNews/NewsHeader.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, TreeMapsPanelComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(BeatLeader::NewsHeaderComponent*, header);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, detailsContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, button);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, image);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, ornamentTease);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, dateText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, topText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, bottomText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, timeText);

    DECLARE_INSTANCE_METHOD(void, Awake);
)

namespace BeatLeader {

class TreeMapsPanel : public ReeUIComponentV2<TreeMapsPanelComponent*> {
public:
    TreeStatus* currentStatus;
    void OnInitialize() override;
    void OnRootStateChange(bool active) override;
    void OnDispose() override;
    StringW GetContent() override;

    std::function<void(bool)> shouldShowOtherMapsEvent;
    void UpdateUI(TreeStatus* status);
    void HandleTreeRequestState(API::RequestState state, TreeStatus* result, StringW failReason);
    void OnPlayButtonPressed();
};

} // namespace BeatLeader 