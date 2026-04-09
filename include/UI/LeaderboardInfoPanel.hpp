#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UI/CaptorClanUI.hpp"
#include "UI/HeaderButton.hpp"
#include "UI/MapTypePanel.hpp"
#include "UI/MapStatus.hpp"
#include "Managers/MapTypesManager.hpp"
#include "shared/Models/Difficulty.hpp"

#include <functional>
#include <optional>
#include <utility>

DECLARE_CLASS_CUSTOM(BeatLeader, LeaderboardInfoPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(BeatLeader::HeaderButtonComponent*, _websiteButton);
    DECLARE_INSTANCE_FIELD(BeatLeader::HeaderButtonComponent*, _settingsButton);
    DECLARE_INSTANCE_FIELD(BeatLeader::MapStatusComponent*, _mapStatus);
    DECLARE_INSTANCE_FIELD(BeatLeader::MapTypePanelComponent*, _mapTypePanel0);
    DECLARE_INSTANCE_FIELD(BeatLeader::MapTypePanelComponent*, _mapTypePanel1);
    DECLARE_INSTANCE_FIELD(BeatLeader::MapTypePanelComponent*, _mapTypePanel2);
    DECLARE_INSTANCE_FIELD(BeatLeader::CaptorClanComponent*, _captorClan);

    DECLARE_INSTANCE_METHOD(void, Awake);
};

namespace BeatLeader {

class LeaderboardInfoPanel : public ReeUIComponentV2<LeaderboardInfoPanelComponent*> {
public:
    void OnInitialize() override;
    void OnDispose() override;
    StringW GetContent() override;

    void SetActive(bool value);
    void SetCallbacks(std::function<void()> websiteCallback, std::function<void()> settingsCallback);
    void SetMapStatusHoverCallback(std::function<void(UnityEngine::Vector3 const&, bool, float)> callback);
    void ClearBeatmapData();
    void SetMapDifficulty(Difficulty const& difficulty);
    void RefreshMapStatus();

private:
    HeaderButton* GetWebsiteButton();
    HeaderButton* GetSettingsButton();
    MapStatus* GetMapStatusView();
    MapTypePanel* GetMapTypePanel(int index);
    CaptorClanView* GetCaptorClanView();
    void ApplyButtonSetup();
    void ApplyBeatmapState();
    void ApplyMapTypes();

    std::function<void()> _websiteCallback;
    std::function<void()> _settingsCallback;
    std::optional<Difficulty> _difficulty;
    int _mapTypesCallbackId = 0;
};

}
