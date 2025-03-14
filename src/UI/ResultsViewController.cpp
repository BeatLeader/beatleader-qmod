#include "UI/ResultsViewController.hpp"

#include "main.hpp"
#include "Utils/ModConfig.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "System/Action.hpp"
#include "Assets/BundleLoader.hpp"
#include "Core/ReplayPlayer.hpp"
#include "Utils/ReplayManager.hpp"
#include "UI/LeaderboardUI.hpp"
#include "UI/QuestUI.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "custom-types/shared/delegate.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "bsml/shared/Helpers/getters.hpp"

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
            auto votingButtonImage = ::BSML::Lite::CreateClickableImage(transform, BundleLoader::bundle->modifiersIcon, [transform]() {
                if (resultsVotingButton->state != 2) return;
                
                if (votingUI == NULL) {
                    // Init voting modal
                    BeatLeader::initVotingPopup(&votingUI, transform, LeaderboardUI::voteCallback);
                }

                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            }, {-67, 9}, {8, 8});
            resultsVotingButton = self->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            resultsVotingButton->Init(votingButtonImage);

            // If we have replay, also show the replay button
            if(ReplayInstalled()) {
                replayButton = QuestUI::CreateUIButton(transform, "", "PracticeButton", {-46, -19}, {12, 10}, []() {
                    // Dont crash if file doesnt exist yet
                    if(std::filesystem::exists(ReplayManager::lastReplayFilename)) {
                        auto flow = BSML::Helpers::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
                        flow->DismissViewController(flow->get_topViewController(), HMUI::ViewController::AnimationDirection::Vertical, custom_types::MakeDelegate<System::Action *>((std::function<void()>)[flow]() {
                            if (flow.try_cast<GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator>()) {
                                flow.cast<GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator>()->SinglePlayerLevelSelectionFlowCoordinatorDidActivate(false, false);
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
                auto *image = QuestUI::CreateImage(replayButton->get_transform(), BundleLoader::bundle->replayIcon);
                image->get_rectTransform()->set_localScale({0.64f, 0.8f, 1.0f});
            }
        }

        // Ajust position based on result screen type (position is different between failure and success)
        if(replayButton) {
            replayButton->get_transform().cast<RectTransform>()->set_anchoredPosition(self->_levelCompletionResults->levelEndStateType == LevelCompletionResults::LevelEndStateType::Cleared ? UnityEngine::Vector2(-46, -30) : UnityEngine::Vector2(-46, -19));
        }

        // Load initial status
        LeaderboardUI::updateVotingButton();
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, GetLastReplayStateHook);
        INSTALL_HOOK(BeatLeaderLogger, ResultsViewDidActivate);
    }

    void reset() {
        resultsVotingButton = NULL;
        replayButton = NULL;
        votingUI = NULL;
    }
}