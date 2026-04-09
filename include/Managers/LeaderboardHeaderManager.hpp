#pragma once

#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UI/MapDifficultyPanel.hpp"
#include "UI/LeaderboardInfoPanel.hpp"
#include "UnityEngine/GameObject.hpp"
#include "shared/Models/Song.hpp"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>

namespace BeatLeader {

class LeaderboardHeaderManager {
public:
    void EnsureInitialized(GlobalNamespace::PlatformLeaderboardViewController* controller,
                           std::function<void()> websiteCallback,
                           std::function<void()> settingsCallback);
    void ApplyVoteToCurrentBeatmap(bool rankable, int type);
    void SetBeatmap(GlobalNamespace::BeatmapKey beatmapKey);
    void SetVisible(bool visible);
    void RefreshMapStatus();
    void Reset();

private:
    bool TryFindHeader(GlobalNamespace::PlatformLeaderboardViewController* controller, UnityEngine::GameObject*& header);
    bool TryGetBeatmapLookupKey(GlobalNamespace::BeatmapKey beatmapKey, std::pair<std::string, std::string>& key) const;
    void ApplyColors(bool funny);
    void ApplyCurrentBeatmapData();
    void ClearCurrentBeatmapData();

    UnityEngine::GameObject* _header = nullptr;
    HMUI::ImageView* _headerImage = nullptr;
    TMPro::TextMeshProUGUI* _headerText = nullptr;
    BeatLeader::MapDifficultyPanel* _mapDifficultyPanel = nullptr;
    BeatLeader::LeaderboardInfoPanel* _infoPanel = nullptr;
    std::map<std::string, Song> _mapInfos;
    std::optional<std::pair<std::string, std::string>> _currentLookupKey;
    int _requestId = 0;
};

namespace LeaderboardHeaderManagerNS {
    extern LeaderboardHeaderManager Instance;
}

}
