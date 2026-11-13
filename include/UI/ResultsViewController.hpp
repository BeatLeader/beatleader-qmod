#pragma once

#include "beatsaber-hook/shared/safeptr.hpp"

#include "UI/VotingButton.hpp"
#include "UI/VotingUI.hpp"

namespace ResultsView {
    extern safe_ptr<BeatLeader::VotingButton*> resultsVotingButton;
    extern BeatLeader::RankVotingPopup* votingUI;
    void setup();
    void reset();
}