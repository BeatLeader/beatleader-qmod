#pragma once

#include "UnityEngine/MonoBehaviour.hpp"

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/TableView.hpp"
#include "HMUI/TableCell.hpp"
#include "System/Object.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "bsml/shared/BSML/Components/TableView.hpp"
#include "bsml/shared/BSML/Components/Backgroundable.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, PreferencesViewController, HMUI::ViewController) {
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHeirarchy, bool screenSystemDisabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidDeactivate, &HMUI::ViewController::DidDeactivate, bool removedFromHierarchy, bool screenSystemDisabling);
};