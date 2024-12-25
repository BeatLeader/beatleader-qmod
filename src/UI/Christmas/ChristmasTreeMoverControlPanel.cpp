#include "UI/Christmas/ChristmasTreeMoverControlPanel.hpp"
#include "API/RequestManager.hpp"
#include "logging.hpp"
#include "Utils/UnityExtensions.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeMoverControlPanelComponent);

namespace BeatLeader {

    void ChristmasTreeMoverControlPanelComponent::SetUploading(bool loading) {
        loadingIndicator->SetActive(loading);
        background->SetActive(!loading);
        uploading = loading;
    }

    void ChristmasTreeMoverControlPanelComponent::HandleFinishButtonClicked() {
        if (uploading) return;
        auto localPos = UnityExtensions::GetLocalPose(tree->get_transform());

        auto pos = FullSerializablePose(localPos.position, tree->get_transform()->get_localScale(), localPos.rotation);

        API::RequestManager::SendUpdateTreeRequest(pos);
    }

    void ChristmasTreeMoverControlPanelComponent::HandleResetButtonClicked() {
        auto defaultPos = UnityEngine::Vector3(2.7f, 0.0f, 4.0f);
        auto defaultRot = UnityEngine::Quaternion::get_identity();
        auto defaultScale = UnityEngine::Vector3(1.7f, 1.7f, 1.7f);

        UnityExtensions::SetLocalPose(tree->get_transform(), FullSerializablePose(defaultPos, defaultScale, defaultRot));
        tree->get_transform()->set_localScale(defaultScale);
    }

    void ChristmasTreeMoverControlPanel::OnInitialize() {
        LocalComponent()->SetUploading(false);
        LocalComponent()->_resetButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                LocalComponent()->HandleResetButtonClicked();
            }
        ));
        LocalComponent()->_finishButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
            (std::function<void()>)[this] { 
                LocalComponent()->HandleFinishButtonClicked();
            }
        ));
        API::RequestManager::AddTreeUploadRequestListener(
            std::bind(&ChristmasTreeMoverControlPanel::OnTreeRequestStateChanged, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void ChristmasTreeMoverControlPanel::OnDispose() {
        API::RequestManager::RemoveTreeUploadRequestListener(
            std::bind(&ChristmasTreeMoverControlPanel::OnTreeRequestStateChanged, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void ChristmasTreeMoverControlPanel::SetupTree(ChristmasTree* tree) {
        LocalComponent()->tree = tree;
    }

    void ChristmasTreeMoverControlPanel::Present() {
        _initialPose = UnityExtensions::GetLocalPose(LocalComponent()->tree->get_transform());
        _initialScale = LocalComponent()->tree->get_transform()->get_localScale();

        LocalComponent()->modal->Show(true, true, nullptr);
        LocalComponent()->tree->SetMoverFull(true);
    }

    void ChristmasTreeMoverControlPanel::Dismiss() {
        LocalComponent()->modal->Hide(true, nullptr);
        LocalComponent()->tree->SetMoverFull(false);
    }

    void ChristmasTreeMoverControlPanel::OnTreeRequestStateChanged(API::RequestState state, StringW result, StringW failReason) {
        switch (state) {
            case API::RequestState::Started:
                LocalComponent()->SetUploading(true);
                break;

            case API::RequestState::Finished:
                LocalComponent()->SetUploading(false);
                API::RequestManager::SendChristmasTreeRequest();
                Dismiss();
                break;

            case API::RequestState::Failed:
                LocalComponent()->SetUploading(false);
                UnityExtensions::SetLocalPose(LocalComponent()->tree->get_transform(), _initialPose);
                LocalComponent()->tree->get_transform()->set_localScale(_initialScale);
                Dismiss();
                break;
        }
    }

    StringW ChristmasTreeMoverControlPanel::GetContent() {
        return StringW(R"(
            <modal id="modal" size-delta-x="54" size-delta-y="18">
                <vertical id="background" spacing="-2">
                    <text text="You are now able to move the tree." align="Center" font-size="3.5"/>
                    <horizontal spacing="1">
                        <button id="_resetButton" text="Reset" pref-height="8"/>
                        <button id="_finishButton" text="Done" pref-height="8"/>
                    </horizontal>
                </vertical>
                <bg id="loadingIndicator" size-delta-x="42" size-delta-y="14">
                    <loading-indicator size-delta-x="12" size-delta-y="12"/>
                </bg>
            </modal> 
        )");
    }

} // namespace BeatLeader 