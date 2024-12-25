#include "UI/Christmas/ChristmasTreeSettingsPanel.hpp"
#include "UI/Christmas/ChristmasTreeManager.hpp"
#include "Assets/BundleLoader.hpp"
#include "Utils/ModConfig.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeSettingsPanelComponent);

namespace BeatLeader {

    void ChristmasTreeSettingsPanelComponent::Awake() {
        moverControlPanel = ChristmasTreeMoverControlPanel::Instantiate<ChristmasTreeMoverControlPanel>(get_transform())->LocalComponent();
        editorButton = HeaderButton::Instantiate<HeaderButton>(get_transform())->LocalComponent();
        movementButton = HeaderButton::Instantiate<HeaderButton>(get_transform())->LocalComponent();

        treeEditor = UnityEngine::GameObject::New_ctor("ChristmasTreeEditor")->AddComponent<ChristmasTreeEditor*>();
        // treeEditor->EditorClosedEvent += std::bind(&ChristmasTreeSettingsPanelComponent::HandleEditorClosed, this);

        editorButton->SetupIcon(BundleLoader::bundle->TreeIcon);
        editorButton->_buttonAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
            HandleEditorButtonClicked();
        });
        movementButton->SetupIcon(BundleLoader::bundle->locationIcon);
        movementButton->_buttonAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
            HandleMoverButtonClicked();
        });
    }

    void ChristmasTreeSettingsPanelComponent::OnDestroy() {
        // Clean up
    }

    void ChristmasTreeSettingsPanelComponent::SetupTree(ChristmasTree* tree) {
        treeEditor->SetupTree(tree);
        moverControlPanel->tree = tree;
    }

    void ChristmasTreeSettingsPanelComponent::Present() {
        if (treeEditor->isOpened) return;
        screen->Present();
    }

    void ChristmasTreeSettingsPanelComponent::Dismiss() {
        screen->Dismiss();
    }

    void ChristmasTreeSettingsPanelComponent::OnClosePressed() {
        Dismiss();
    }

    void ChristmasTreeSettingsPanelComponent::HandleEditorButtonClicked() {
        treeEditor->Present();
        Dismiss();
    }

    void ChristmasTreeSettingsPanelComponent::HandleMoverButtonClicked() {
        auto moverNative = moverControlPanel->nativeComponent;
        if (moverNative) {
            auto mover = reinterpret_cast<ChristmasTreeMoverControlPanel*>(moverNative);
            mover->Present();
        }
    }

    void ChristmasTreeSettingsPanelComponent::HandleEditorClosed() {
        Present();
    }

    void ChristmasTreeSettingsPanelComponent::HandleChristmasSettingsUpdated() {
        if (buttonsContainer) {
            buttonsContainer->SetActive(getModConfig().TreeEnabled.GetValue());
        }
    }

    void ChristmasTreeSettingsPanelComponent::set_EnableTree(bool value) {
        getModConfig().TreeEnabled.SetValue(value);
        ChristmasTreeManager::HandleChristmasSettingsUpdated();
        HandleChristmasSettingsUpdated();
    }

    bool ChristmasTreeSettingsPanelComponent::get_EnableTree() {
        return getModConfig().TreeEnabled.GetValue();
    }

    void ChristmasTreeSettingsPanelComponent::set_EnableSnow(bool value) {
        getModConfig().SnowEnabled.SetValue(value);
        ChristmasTreeManager::HandleChristmasSettingsUpdated();
        HandleChristmasSettingsUpdated();
    }

    bool ChristmasTreeSettingsPanelComponent::get_EnableSnow() {
        return getModConfig().SnowEnabled.GetValue();
    }

    void ChristmasTreeSettingsPanelComponent::set_EnableOthersTree(bool value) {
        getModConfig().OthersTreeEnabled.SetValue(value);
        ChristmasTreeManager::HandleChristmasSettingsUpdated();
        HandleChristmasSettingsUpdated();
    }

    bool ChristmasTreeSettingsPanelComponent::get_EnableOthersTree() {
        return getModConfig().OthersTreeEnabled.GetValue();
    }

    void ChristmasTreeSettingsPanel::OnInitialize() {
        LocalComponent()->screen = LocalComponent()->_content->get_gameObject()->AddComponent<StaticScreen*>();
        LocalComponent()->_content->set_position(UnityEngine::Vector3(0.0f, 0.9f, 0.7f));
        LocalComponent()->_content->set_eulerAngles(UnityEngine::Vector3(45.0f, 0.0f, 0.0f));

        LocalComponent()->HandleChristmasSettingsUpdated();
        // Initialize component
    }

    void ChristmasTreeSettingsPanel::OnInstantiate() {
        
    }

    void ChristmasTreeSettingsPanel::OnDispose() {
        // Clean up
    }

    StringW ChristmasTreeSettingsPanel::GetContent() {
        return StringW(R"(
            <bg size-delta-x="42" size-delta-y="29">
                <vertical pref-width="42" pref-height="29" vertical-fit="PreferredSize">
                    <macro.as-host host="moverControlPanel">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>
                    <vertical pref-height="25" bg="round-rect-panel" pad-left="2" pad-right="2" horizontal-fit="Unconstrained">
                        <toggle-setting text="Enable Tree" value="EnableTree" bind-value="true" apply-on-change="true"/>
                        <toggle-setting text="Enable Snow" value="EnableSnow" bind-value="true" apply-on-change="true"/>
                        <toggle-setting text="Other's trees" hover-hint="Show trees of other players on score details" value="EnableOthersTree" bind-value="true" apply-on-change="true"/>
                    </vertical>
                    <horizontal pref-height="8">
                        <button text="Close" on-click="OnClosePressed" pref-width="20"/>
                        <horizontal id="buttonsContainer" pref-height="6" pad="1" bg="round-rect-panel">
                            <macro.as-host host="editorButton">
                                <macro.reparent transform="_uiComponent"/>
                            </macro.as-host>
                            <macro.as-host host="movementButton">
                                <macro.reparent transform="_uiComponent"/>
                            </macro.as-host>
                        </horizontal>
                    </horizontal>
                </vertical>
            </bg>
        )");
    }

} // namespace BeatLeader 