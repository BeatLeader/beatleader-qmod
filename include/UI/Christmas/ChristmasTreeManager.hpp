#pragma once

#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Christmas/ChristmasTreeSettingsPanel.hpp"
#include "UI/Christmas/SnowController.hpp"
#include "API/RequestManager.hpp"
#include "Models/ChristmasSettings.hpp"
#include "Models/ChristmasTreeSettings.hpp"
#include "Models/Score.hpp"
#include "Utils/ModConfig.hpp"

namespace BeatLeader {

class ChristmasTreeManager {
public:
    static void Initialize();
    static void LateDispose();
    static void SpawnTree();
    static void SpawnSnow();

    static void HandleTreeRequestState(API::RequestState state, ChristmasTreeSettings* settings, StringW failReason);
    static void HandleOthersTreeRequestState(API::RequestState state, ChristmasTreeSettings* settings, StringW failReason);
    static void HandleScoreInfoPanelVisibility(bool visible, Score* score);
    static void HandleChristmasSettingsUpdated();
    static void HandleCoordinatorPresented();
    static void HandleTreeButtonClicked();
    static void HandleTreeEditorWasRequested();

    static bool CanPresentTree() {
        return 
        // !settingsPanel->IsEditorOpened() &&  treeEditor->isOpened;
               getModConfig().TreeEnabled.GetValue() && 
               treeSettingsLoaded;
    }

    static inline ChristmasTree* christmasTree = nullptr;
    static inline ChristmasTreeSettingsPanel* settingsPanel = nullptr;
    static inline SnowController* snow = nullptr;
    static inline ChristmasTree* othersTree = nullptr;
    static inline bool treeSettingsLoaded = false;
    static inline bool coordinatorWasPresented = false;
};

} // namespace BeatLeader 