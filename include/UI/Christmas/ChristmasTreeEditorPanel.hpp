#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "HMUI/ViewController.hpp"
#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "System/Action.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, ChristmasTreeEditorPanelComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _bg);
    DECLARE_INSTANCE_FIELD(StaticScreen*, _screen);
    DECLARE_INSTANCE_FIELD(UnityEngine::CanvasGroup*, bgGroup);
    DECLARE_INSTANCE_FIELD(bool, isUploading);
    DECLARE_INSTANCE_FIELD(ChristmasTree*, tree);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _cancelButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _clearButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _saveButton);

    DECLARE_INSTANCE_FIELD(System::Action*, cancelAction);
    DECLARE_INSTANCE_FIELD(System::Action*, saveAction);
    DECLARE_INSTANCE_FIELD(System::Action*, clearAction);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, SetLoading, bool loading);
    DECLARE_INSTANCE_METHOD(void, HandleClearClicked);
    
    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);

    public:
        void SetupTree(ChristmasTree* tree);
)

namespace BeatLeader {

class ChristmasTreeEditorPanel : public ReeUIComponentV2<ChristmasTreeEditorPanelComponent*> {
public:
    void OnInitialize() override;

    StringW GetContent() override;
};

} // namespace BeatLeader 