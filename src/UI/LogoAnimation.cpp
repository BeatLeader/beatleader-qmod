#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "API/PlayerController.hpp"

#include "include/UI/LogoAnimation.hpp"

#include "main.hpp"

#include <string>

using namespace QuestUI;
using namespace std;

DEFINE_TYPE(BeatLeader, LogoAnimation);

void BeatLeader::LogoAnimation::Update() {
    
}