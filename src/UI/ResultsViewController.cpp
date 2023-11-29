#include "UI/ResultsViewController.hpp"

#include "main.hpp"
#include "Utils/ModConfig.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "System/Action.hpp"
#include "Assets/BundleLoader.hpp"
#include "Core/ReplayPlayer.hpp"
#include "Utils/ReplayManager.hpp"
#include "UI/LeaderboardUI.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "custom-types/shared/delegate.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include <filesystem>

using namespace GlobalNamespace;

namespace ResultsView {

    SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
    SafePtrUnity<UnityEngine::UI::Button> replayButton;
    BeatLeader::RankVotingPopup* votingUI;
    bool lastGameWasReplay = false;

    MAKE_HOOK_MATCH(GetLastReplayStateHook, &MenuTransitionsHelper::HandleMainGameSceneDidFinish, void, MenuTransitionsHelper* self, StandardLevelScenesTransitionSetupDataSO* standardLevelScenesTransitionSetupData, LevelCompletionResults* levelCompletionResults){
        // Save if the last Game was a Replay
        if(ReplayInstalled()) {
            lastGameWasReplay = IsInReplay();
        }
        GetLastReplayStateHook(self, standardLevelScenesTransitionSetupData, levelCompletionResults);
    }

    MAKE_HOOK_MATCH(ResultsViewDidActivate, &ResultsViewController::DidActivate, void, ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        ResultsViewDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

        if(firstActivation){
            auto transform = self->get_transform();
            // Create voting button
            auto votingButtonImage = ::QuestUI::BeatSaberUI::CreateClickableImage(transform, BundleLoader::bundle->modifiersIcon, {-67, 9}, {8, 8}, [transform]() {
                if (resultsVotingButton->state != 2) return;
                
                if (votingUI == NULL) {
                    // Init voting modal
                    BeatLeader::initVotingPopup(&votingUI, transform, LeaderboardUI::voteCallback);
                }

                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            resultsVotingButton = self->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            resultsVotingButton->Init(votingButtonImage);

            // If we have replay, also show the replay button
            if(ReplayInstalled()) {
                replayButton = QuestUI::BeatSaberUI::CreateUIButton(transform, "", "PracticeButton", {-46, -19}, {12, 10}, []() {
                    // Dont crash if file doesnt exist yet
                    if(std::filesystem::exists(ReplayManager::lastReplayFilename)) {
                        auto flow = QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
                        flow->DismissViewController(flow->get_topViewController(), HMUI::ViewController::AnimationDirection::Vertical, custom_types::MakeDelegate<System::Action *>(classof(System::Action *), (std::function<void()>)[flow]() {
                            if (il2cpp_utils::try_cast<GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator>(flow)) {
                                ((GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator *)flow)->SinglePlayerLevelSelectionFlowCoordinatorDidActivate(false, false);
                            }
                            if(!lastGameWasReplay || !getModConfig().ShowReplaySettings.GetValue()){
                                if (getModConfig().ShowReplaySettings.GetValue()) {
                                    PlayReplayFromFile(ReplayManager::lastReplayFilename);
                                }
                                else {
                                    PlayReplayFromFileWithoutSettings(ReplayManager::lastReplayFilename);
                                }
                            }
                        }), true);
                    }
                });
                // Set icon of button
                auto *image = QuestUI::BeatSaberUI::CreateImage(replayButton->get_transform(), BundleLoader::bundle->replayIcon);
                image->get_rectTransform()->set_localScale({0.64f, 0.8f, 1.0f});
            }
        }

        // Ajust position based on result screen type (position is different between failure and success)
        if(replayButton) {
            ((RectTransform*)replayButton->get_transform())->set_anchoredPosition(self->levelCompletionResults->levelEndStateType == LevelCompletionResults::LevelEndStateType::Cleared ? UnityEngine::Vector2(-46, -30) : UnityEngine::Vector2(-46, -19));
        }

        // Load initial status
        LeaderboardUI::updateVotingButton();
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, GetLastReplayStateHook);
        INSTALL_HOOK(logger, ResultsViewDidActivate);
    }
}