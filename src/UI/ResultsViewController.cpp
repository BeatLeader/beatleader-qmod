#include "UI/ResultsViewController.hpp"

#include "main.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "Assets/BundleLoader.hpp"
#include "Core/ReplayPlayer.hpp"
#include "Utils/ReplayManager.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

#include <filesystem>

using namespace GlobalNamespace;

namespace ResultsView {

    SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
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
                auto replayButton = QuestUI::BeatSaberUI::CreateUIButton(self->get_transform(), "", "PracticeButton", {-46, -19}, {12, 10}, []() {
                    // Dont crash if file doesnt exist yet
                    if(std::filesystem::exists(ReplayManager::lastReplayFilename)) {
                        PlayReplayFromFile(ReplayManager::lastReplayFilename);
                    }
                });
                QuestUI::BeatSaberUI::SetButtonIcon(replayButton, BundleLoader::bundle->replayIcon);
            }

            getLogger().info("Created Voting Button");
        }
        resultsVotingButton->get_gameObject()->SetActive(true);
        getLogger().info("Activated Voting Button");
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, ResultsViewDidActivate);
    }
}