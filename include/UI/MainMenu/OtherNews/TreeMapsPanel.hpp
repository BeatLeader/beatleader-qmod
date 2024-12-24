#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/ImageView.hpp"
// #include "Models/TreeStatus.hpp"
// #include "API/RequestManager.hpp"

#include "UI/MainMenu/OtherNews/NewsHeader.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, TreeMapsPanelComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(BeatLeader::NewsHeaderComponent*, header);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _detailsContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _actionButton);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _songCover);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _ornamentTease);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _dateText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _topText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _bottomText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _timeText);

    DECLARE_INSTANCE_METHOD(void, Awake);
)

namespace BeatLeader {

class TreeMapsPanel : public ReeUIComponentV2<TreeMapsPanelComponent*> {
public:
    // TreeStatus* currentStatus;
    void OnInitialize() override;
    // void OnRootStateChange(bool active) override;
    // void OnDispose() override;
    StringW GetContent() override;

    // std::function<void(bool)> shouldShowOtherMapsEvent;
    // void UpdateUI(TreeStatus* status);
    // void HandleTreeRequestState(API::RequestState state, TreeStatus* result, StringW failReason);
    void OnPlayButtonPressed();
};

} // namespace BeatLeader 