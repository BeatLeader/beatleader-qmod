#include "UI/ResultsViewController.hpp"

#include "main.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "Assets/BundleLoader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

using namespace GlobalNamespace;

namespace ResultsView {

    SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
    BeatLeader::RankVotingPopup* votingUI;

    MAKE_HOOK_MATCH(ResultsViewDidActivate, &ResultsViewController::DidActivate, void, ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        ResultsViewDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

        if(firstActivation){
            getLogger().info("InitedModal Voting Button");
            BeatLeader::initVotingPopup(&votingUI, self->get_transform(), LeaderboardUI::voteCallback);
            getLogger().info("Creating Voting Button");
            auto votingButtonImage = ::QuestUI::BeatSaberUI::CreateClickableImage(self->get_transform(), BundleLoader::bundle->modifiersIcon, UnityEngine::Vector2(-65, -2), UnityEngine::Vector2(8, 8), []() {
                if (resultsVotingButton->state != 2) return;
                
                votingUI->reset();
                votingUI->modal->Show(true, true, nullptr);
            });
            resultsVotingButton = self->get_gameObject()->AddComponent<BeatLeader::VotingButton*>();
            resultsVotingButton->Init(votingButtonImage);

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