#pragma once

#include "HMUI/ViewController.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/Color.hpp"

#include "include/Models/Score.hpp"

#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"

#include "custom-types/shared/macros.hpp"

extern GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator* levelSelectCoordinator;

DECLARE_CLASS_CODEGEN(BeatLeader, ScoreDetails, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    public:
    void setScore(Score score);
)