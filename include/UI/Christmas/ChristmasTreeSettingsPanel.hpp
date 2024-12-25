#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Christmas/ChristmasTreeMoverControlPanel.hpp"
#include "UI/Christmas/ChristmasTreeEditor.hpp"
#include "UI/Components/HeaderButton.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "Utils/ModConfig.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, ChristmasTreeSettingsPanelComponent, BeatLeader::ReeComponent,
    DECLARE_INSTANCE_FIELD(HeaderButtonComponent*, editorButton);
    DECLARE_INSTANCE_FIELD(HeaderButtonComponent*, movementButton);
    DECLARE_INSTANCE_FIELD(ChristmasTreeMoverControlPanelComponent*, moverControlPanel);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, buttonsContainer);
    DECLARE_INSTANCE_FIELD(ChristmasTreeEditor*, treeEditor);
    DECLARE_INSTANCE_FIELD(StaticScreen*, screen);

    DECLARE_INSTANCE_METHOD(void, set_EnableTree, bool value);
    DECLARE_INSTANCE_METHOD(bool, get_EnableTree);
    DECLARE_INSTANCE_METHOD(void, set_EnableSnow, bool value);
    DECLARE_INSTANCE_METHOD(bool, get_EnableSnow);
    DECLARE_INSTANCE_METHOD(void, set_EnableOthersTree, bool value);
    DECLARE_INSTANCE_METHOD(bool, get_EnableOthersTree);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, OnClosePressed);
    DECLARE_INSTANCE_METHOD(void, HandleEditorButtonClicked);
    DECLARE_INSTANCE_METHOD(void, HandleMoverButtonClicked);
    DECLARE_INSTANCE_METHOD(void, HandleEditorClosed);
    DECLARE_INSTANCE_METHOD(void, HandleChristmasSettingsUpdated);
    DECLARE_INSTANCE_METHOD(void, SetupTree, ChristmasTree* tree);
    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);
)

namespace BeatLeader {

class ChristmasTreeSettingsPanel : public ReeUIComponentV2<ChristmasTreeSettingsPanelComponent*> {
public:
    void OnInitialize() override;
    void OnInstantiate() override;
    void OnDispose() override;
    StringW GetContent() override;
};

} // namespace BeatLeader 