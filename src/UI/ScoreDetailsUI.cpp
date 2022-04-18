#include "main.hpp"
#include "include/UI/ScoreDetailsUI.hpp"

#include "questui/shared/BeatSaberUI.hpp"

#include "HMUI/ViewController_AnimationType.hpp"

#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/CustomPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/LeaderboardViewController.hpp"

#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/Tasks/Task_1.hpp"

#include "UnityEngine/UI/Image_Origin360.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Material.hpp"

#include <iomanip>
#include <sstream>

DEFINE_TYPE(BeatLeader, ScoreDetails);

using namespace QuestUI;
using namespace GlobalNamespace;
using namespace BeatLeader;

void ScoreDetails::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
   // modal = CreateModal(parent, UnityEngine::Vector2(60, x), [](HMUI::ModalView *modal) {}, true);
}

void ScoreDetails::setScore(Score score) {
    
}