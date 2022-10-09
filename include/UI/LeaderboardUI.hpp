#pragma once

#include <string>
#include "include/Utils/ReplayManager.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/Vector2.hpp"

using namespace std;

namespace LeaderboardUI {
    extern function<void()> retryCallback; 

    void setup();
    void reset();

    void updateStatus(ReplayUploadStatus status, string description, float progress, bool showRestart);
    void initSettingsModal(UnityEngine::Transform* parent);
    UnityEngine::UI::Toggle* CreateToggle(UnityEngine::Transform* parent, bool currentValue, UnityEngine::Vector2 anchoredPosition, std::function<void(bool)> onValueChange);
}