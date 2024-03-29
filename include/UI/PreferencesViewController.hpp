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

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::methodInfo()


DECLARE_CLASS_CODEGEN(BeatLeader, PreferencesViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, GET_FIND_METHOD(&HMUI::ViewController::DidActivate), bool firstActivation, bool addedToHeirarchy, bool screenSystemDisabling);
    DECLARE_OVERRIDE_METHOD(void, DidDeactivate, GET_FIND_METHOD(&HMUI::ViewController::DidDeactivate), bool removedFromHierarchy, bool screenSystemDisabling);
)