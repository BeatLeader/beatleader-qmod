#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/ModalView.hpp"
#include "ChristmasTree.hpp"
#include "Models/SerializablePose.hpp"
#include "API/RequestManager.hpp"
#include "UnityEngine/UI/Button.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, ChristmasTreeMoverControlPanelComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, background);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, modal);
    DECLARE_INSTANCE_FIELD(ChristmasTree*, tree);
    DECLARE_INSTANCE_FIELD(bool, uploading);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _resetButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _finishButton);

    DECLARE_INSTANCE_METHOD(void, SetUploading, bool loading);
    DECLARE_INSTANCE_METHOD(void, HandleFinishButtonClicked);
    DECLARE_INSTANCE_METHOD(void, HandleResetButtonClicked);
)

namespace BeatLeader {

class ChristmasTreeMoverControlPanel : public ReeUIComponentV2<ChristmasTreeMoverControlPanelComponent*> {
public:
    void OnInitialize() override;
    void OnDispose() override;
    StringW GetContent() override;
    void SetupTree(ChristmasTree* tree);
    void Present();
    void Dismiss();

private:
    
    UnityEngine::Pose _initialPose;
    UnityEngine::Vector3 _initialScale;

    void OnTreeRequestStateChanged(API::RequestState state, StringW result, StringW failReason);
};

} // namespace BeatLeader 