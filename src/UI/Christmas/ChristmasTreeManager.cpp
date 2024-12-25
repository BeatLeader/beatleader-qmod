#include "UI/Christmas/ChristmasTreeManager.hpp"
#include "Assets/BundleLoader.hpp"
#include "API/RequestManager.hpp"
#include "Utils/UnityExtensions.hpp"

namespace BeatLeader {

    void ChristmasTreeManager::Initialize() {
        SpawnTree();
        SpawnSnow();

        API::RequestManager::AddTreeRequestListener(&ChristmasTreeManager::HandleTreeRequestState);
        API::RequestManager::SendChristmasTreeRequest();

        API::RequestManager::AddOthersTreeRequestListener(&ChristmasTreeManager::HandleOthersTreeRequestState);

        // LeaderboardEvents::TreeButtonWasPressedEvent += &ChristmasTreeManager::HandleTreeButtonClicked;
        // LeaderboardEvents::TreeEditorWasRequestedEvent += &ChristmasTreeManager::HandleTreeEditorWasRequested;
        // SoloFlowCoordinatorPatch::PresentedEvent += &ChristmasTreeManager::HandleCoordinatorPresented;
        // PluginConfig::ChristmasSettingsUpdatedEvent += &ChristmasTreeManager::HandleChristmasSettingsUpdated;
        // ScoreInfoPanel::ScoreInfoPanelEvent += &ChristmasTreeManager::HandleScoreInfoPanelVisibility;
    }

    void ChristmasTreeManager::LateDispose() {
        // API::RequestManager::RemoveTreeRequestListener(&ChristmasTreeManager::HandleTreeRequestState);

        // LeaderboardEvents::TreeButtonWasPressedEvent -= &ChristmasTreeManager::HandleTreeButtonClicked;
        // LeaderboardEvents::TreeEditorWasRequestedEvent -= &ChristmasTreeManager::HandleTreeEditorWasRequested;
        // SoloFlowCoordinatorPatch::PresentedEvent -= &ChristmasTreeManager::HandleCoordinatorPresented;
        // PluginConfig::ChristmasSettingsUpdatedEvent -= &ChristmasTreeManager::HandleChristmasSettingsUpdated;
        // ScoreInfoPanel::ScoreInfoPanelEvent -= &ChristmasTreeManager::HandleScoreInfoPanelVisibility;

        // if (christmasTree) {
        //     UnityEngine::Object::Destroy(christmasTree->get_gameObject());
        //     christmasTree = nullptr;
        // }
        // if (settingsPanel) {
        //     UnityEngine::Object::Destroy(settingsPanel->get_gameObject());
        //     settingsPanel = nullptr;
        // }
        // if (snow) {
        //     UnityEngine::Object::Destroy(snow->get_gameObject());
        //     snow = nullptr;
        // }
        // if (othersTree) {
        //     UnityEngine::Object::Destroy(othersTree->get_gameObject());
        //     othersTree = nullptr;
        // }
    }

    void ChristmasTreeManager::SpawnTree() {
        auto prefab = BundleLoader::bundle->treeContainer->GetComponent<BeatLeader::ChristmasTree*>();
        christmasTree = UnityEngine::Object::Instantiate(prefab, nullptr, false);
    }

    void ChristmasTreeManager::SpawnSnow() {
        auto snowController = BundleLoader::bundle->snowContainer->GetComponent<BeatLeader::SnowController*>();
        snow = Object::Instantiate(snowController, nullptr, false);

        if (getModConfig().SnowEnabled.GetValue()) {
            snow->Play(true);
        } else {
            snow->Stop();
        }
    }

    void ChristmasTreeManager::HandleTreeRequestState(API::RequestState state, ChristmasTreeSettings* settings, StringW failReason) {
        if (state != API::RequestState::Finished || !settings) return;

        christmasTree->LoadSettings(*settings, true);
        treeSettingsLoaded = true;
        if (CanPresentTree()) {
            christmasTree->Present();
        }
    }

    void ChristmasTreeManager::HandleOthersTreeRequestState(API::RequestState state, ChristmasTreeSettings* settings, StringW failReason) {
        if (state != API::RequestState::Finished || !settings) return;

        if (othersTree) {
            othersTree->LoadSettings(*settings, false);
            othersTree->ScaleTo(1.4f, false);
            othersTree->StartSpinning();
        }
    }

    void ChristmasTreeManager::HandleScoreInfoPanelVisibility(bool visible, Score* score) {
        if (!getModConfig().OthersTreeEnabled.GetValue()) return;

        if (visible) {
            if (!othersTree) {
                auto prefab = BundleLoader::bundle->treeContainer->GetComponent<BeatLeader::ChristmasTree*>();
                othersTree = UnityEngine::Object::Instantiate(prefab, nullptr, false);

                UnityExtensions::SetLocalPose(othersTree->get_transform(), FullSerializablePose(
                    UnityEngine::Vector3(4.2f, 0.0f, 0.5f),
                    UnityEngine::Vector3(1.0f, 1.0f, 1.0f),
                    UnityEngine::Quaternion::get_identity()
                ));
                othersTree->Dismiss();
            }
            API::RequestManager::SendOthersTreeRequest(score->player.id);
        } else {
            if (othersTree) {
                othersTree->Dismiss();
            }
        }
    }

    void ChristmasTreeManager::HandleChristmasSettingsUpdated() {
        if (snow) {
            if (getModConfig().SnowEnabled.GetValue()) {
                snow->Play(false);
            } else {
                snow->Stop();
            }
        }
        if (christmasTree) {
            if (getModConfig().TreeEnabled.GetValue()) {
                christmasTree->Present();
            } else {
                christmasTree->Dismiss();
            }
        }
    }

    void ChristmasTreeManager::HandleCoordinatorPresented() {
        coordinatorWasPresented = true;
        if (CanPresentTree()) {
            christmasTree->Present();
        }
    }

    void ChristmasTreeManager::HandleTreeButtonClicked() {
        if (!settingsPanel) {
            settingsPanel = ChristmasTreeSettingsPanel::Instantiate<ChristmasTreeSettingsPanel>(nullptr);
            settingsPanel->LocalComponent()->ManualInit(nullptr);
            settingsPanel->LocalComponent()->SetupTree(christmasTree);
        }
        settingsPanel->LocalComponent()->Present();
    }

    void ChristmasTreeManager::HandleTreeEditorWasRequested() {
        settingsPanel->LocalComponent()->Present();
        // settingsPanel->HandleEditorButtonClicked();
    }

} // namespace BeatLeader 