#pragma once

#include "custom-types/shared/macros.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/IconSegmentedControl.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"

#include "GlobalNamespace/LeaderboardTableView.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"

DECLARE_CLASS_CODEGEN(
    LeaderboardUI, LeaderboardViewController, 
    HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(GlobalNamespace::LeaderboardTableView*, leaderboardTableView);
    DECLARE_INSTANCE_FIELD(HMUI::IconSegmentedControl*, scopeSegmentedControl);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, loadingLabel);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::ViewController::DidActivate>::get(), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, DidDeactivate, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::ViewController::DidDeactivate>::get(), bool removedFromHierarchy, bool screenSystemDisabling);

    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnIconSelected, HMUI::IconSegmentedControl* segmentedControl, int index);
)
