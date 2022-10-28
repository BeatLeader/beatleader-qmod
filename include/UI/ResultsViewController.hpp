#pragma once

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "UI/VotingButton.hpp"
#include "UI/VotingUI.hpp"

namespace ResultsView {
    extern SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
    extern BeatLeader::RankVotingPopup* votingUI;
    void setup();
}