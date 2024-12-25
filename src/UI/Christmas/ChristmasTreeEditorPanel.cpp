#include "UI/Christmas/ChristmasTreeEditorPanel.hpp"
#include "UnityEngine/CanvasGroup.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "API/RequestManager.hpp"
#include "main.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeEditorPanelComponent);

namespace BeatLeader {

    void ChristmasTreeEditorPanelComponent::Awake() {
        
    }

    void ChristmasTreeEditorPanelComponent::SetLoading(bool loading) {
        isUploading = loading;
        if (bgGroup) {
            bgGroup->set_alpha(loading ? 0.3f : 1.0f);
            bgGroup->set_interactable(!loading);
        }
        _loadingIndicator->SetActive(loading);
    }

    void ChristmasTreeEditorPanelComponent::HandleClearClicked() {
        if (tree) {
            tree->ClearOrnaments();
        }
    }

    // void ChristmasTreeEditorPanelComponent::HandleCancelClicked() {
    //     auto panel = (ChristmasTreeEditorPanel*)nativeComponent;
    //     if (panel->_onCancelCallback) {
    //         panel->_onCancelCallback();
    //     }
    //     API::RequestManager::SendChristmasTreeRequest();
    // }

    // void ChristmasTreeEditorPanelComponent::HandleSaveClicked() {
    //     if (isUploading) return;
    //     auto panel = (ChristmasTreeEditorPanel*)nativeComponent;
    //     if (panel->_onSaveCallback) {
    //         panel->_onSaveCallback();
    //     }
    // }

    void ChristmasTreeEditorPanel::OnInitialize() {
        LocalComponent()->_screen = LocalComponent()->_content->get_gameObject()->AddComponent<StaticScreen*>();
        // LocalComponent()->_content->set_position(UnityEngine::Vector3(0.0f, 0.9f, 0.7f));
        // LocalComponent()->_content->set_eulerAngles(UnityEngine::Vector3(45.0f, 0.0f, 0.0f));

        LocalComponent()->_cancelButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                if (LocalComponent()->cancelAction) {
                    LocalComponent()->cancelAction->Invoke();
                }
            }
        ));

        LocalComponent()->_clearButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                if (LocalComponent()->clearAction) {
                    LocalComponent()->clearAction->Invoke();
                }
            }
        ));

        LocalComponent()->_saveButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                if (LocalComponent()->saveAction) {
                    LocalComponent()->saveAction->Invoke();
                }
            }
        ));


        LocalComponent()->bgGroup = LocalComponent()->_bg->GetComponent<UnityEngine::CanvasGroup*>();
        auto indicatorGroup = LocalComponent()->_loadingIndicator->AddComponent<UnityEngine::CanvasGroup*>();
        indicatorGroup->set_ignoreParentGroups(true);
        LocalComponent()->SetLoading(false);
    }

    void ChristmasTreeEditorPanelComponent::SetupTree(ChristmasTree* tree) {
        this->tree = tree;
    }

    void ChristmasTreeEditorPanelComponent::Present() {
        _screen->Present();
    }

    void ChristmasTreeEditorPanelComponent::Dismiss() {
        _screen->Dismiss();
    }

    StringW ChristmasTreeEditorPanel::GetContent() {
        return StringW(R"(
            <vertical id="_bg" pref-width="44" pref-height="24" vertical-fit="PreferredSize">
                <horizontal pref-height="16" bg="round-rect-panel">
                    <text text="TIP: Use joysticks on your controllers to rotate the tree" align="Center" word-wrapping="true"/>
                </horizontal>
                <horizontal pref-height="8" spacing="2">
                    <button id="_cancelButton" text="Cancel" on-click="cancel-click" horizontal-fit="Unconstrained"/>
                    <button id="_clearButton" text="Clear" on-click="clear-click" horizontal-fit="Unconstrained"/>
                    <primary-button id="_saveButton" pref-width="16" text="Save" on-click="save-click" horizontal-fit="PreferredSize"/>
                </horizontal>
                <bg id="_loadingIndicator" ignore-layout="true" pref-width="40" pref-height="24">
                    <loading-indicator size-delta-x="14" size-delta-y="14"/>
                </bg>
            </vertical>
        )");
    }

} // namespace BeatLeader 