#pragma once

#include "custom-types/shared/macros.hpp"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, DanceEventResultsPanel, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _image);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _title);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _result);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loading);
};

namespace DanceResultsUI {
    void Setup(GlobalNamespace::ResultsViewController* resultsView, bool lastGameWasReplay);
    void setup();
}
