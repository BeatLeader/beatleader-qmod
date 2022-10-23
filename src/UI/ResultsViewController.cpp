#include "UI/ResultsViewController.hpp"

#include "main.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/LeaderboardUI.hpp"
#include "questui/shared/CustomTypes/Components/ClickableImage.hpp"
#include "UnityEngine/RectTransform.hpp"

using namespace GlobalNamespace;

namespace ResultsView {

    SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;

    MAKE_HOOK_MATCH(ResultsViewDidActivate, &ResultsViewController::DidActivate, void, ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        ResultsViewDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

        if(firstActivation){
            // Clone existing button
            resultsVotingButton = UnityEngine::GameObject::Instantiate(LeaderboardUI::votingButton);
            resultsVotingButton->get_transform()->SetParent(self->get_transform());

            resultsVotingButton->GetComponent<QuestUI::ClickableImage*>()->get_rectTransform()->set_anchoredPosition({-65, 22});
        }
    }

    void setup() {
        LoggerContextObject logger = getLogger().WithContext("load");
        INSTALL_HOOK(logger, ResultsViewDidActivate);
    }
}