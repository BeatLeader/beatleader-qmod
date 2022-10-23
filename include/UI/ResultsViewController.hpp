#pragma once

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "UI/VotingButton.hpp"

namespace ResultsView {
    extern SafePtrUnity<BeatLeader::VotingButton> resultsVotingButton;
    void setup();
}