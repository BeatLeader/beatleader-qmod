#include "UI/ResultsViewController.hpp"

#include "main.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "Assets/BundleLoader.hpp"
#include "Core/ReplayPlayer.hpp"
#include "Utils/ReplayManager.hpp"
#include "UI/LeaderboardUI.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

#include <filesystem>

using namespace GlobalNamespace;

namespace ResultsView {

    SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
    SafePtrUnity<UnityEngine::UI::Button> replayButton;
    BeatLeader::RankVotingPopup* votingUI;

    MAKE_HOOK_MATCH(ResultsViewDidActivate, &ResultsViewController::DidActivate, void, ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        ResultsViewDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

        if(firstActivation){
            // Init voting modal
            BeatLeader::initVotingPopup(&votingUI, self->get_transform(), LeaderboardUI::voteCallback);

            // Create voting button
            auto votingButtonImage = ::QuestUI::BeatSaberUI::CreateClickableImage(self->get_transform(), BundleLoader::bundle->modifiersIcon, {-65, -2}, {8, 8}, []() {
                if (resultsVotingButton->state != 2) return;
                
                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            resultsVotingButton = self->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            resultsVotingButton->Init(votingButtonImage);

            // If we have replay, also show the replay button
            if(ReplayInstalled()) {
                replayButton = QuestUI::BeatSaberUI::CreateUIButton(self->get_transform(), "", "PracticeButton", {-46, -19}, {12, 10}, []() {
                    // Dont crash if file doesnt exist yet
                    if(std::filesystem::exists(ReplayManager::lastReplayFilename)) {
                        PlayReplayFromFile(ReplayManager::lastReplayFilename);
                    }
                });
                // Set icon of button
                auto *image = QuestUI::BeatSaberUI::CreateImage(replayButton->get_transform(), BundleLoader::bundle->replayIcon);
                image->get_rectTransform()->set_localScale({0.64f, 0.8f, 1.0f});
            }
        }

        ((RectTransform*)replayButton->get_transform())->set_anchoredPosition(self->levelCompletionResults->levelEndStateType == LevelCompletionResults::LevelEndStateType::Cleared ? UnityEngine::Vector2(-46, -30) : UnityEngine::Vector2(-46, -19));

        // Load initial status
        LeaderboardUI::updateVotingButton();
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, ResultsViewDidActivate);
    }
}