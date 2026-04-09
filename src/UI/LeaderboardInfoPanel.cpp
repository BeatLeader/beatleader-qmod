#include "include/UI/LeaderboardInfoPanel.hpp"

#include "include/Assets/BundleLoader.hpp"

DEFINE_TYPE(BeatLeader, LeaderboardInfoPanelComponent);

namespace BeatLeader {

    void LeaderboardInfoPanelComponent::Awake() {
        _websiteButton = HeaderButton::Instantiate<HeaderButton>(get_transform())->LocalComponent();
        _settingsButton = HeaderButton::Instantiate<HeaderButton>(get_transform())->LocalComponent();
        _mapStatus = MapStatus::Instantiate<MapStatus>(get_transform())->LocalComponent();
        _mapTypePanel0 = MapTypePanel::Instantiate<MapTypePanel>(get_transform())->LocalComponent();
        _mapTypePanel1 = MapTypePanel::Instantiate<MapTypePanel>(get_transform())->LocalComponent();
        _mapTypePanel2 = MapTypePanel::Instantiate<MapTypePanel>(get_transform())->LocalComponent();
        _captorClan = CaptorClanView::Instantiate<CaptorClanView>(get_transform())->LocalComponent();
    }

    void LeaderboardInfoPanel::OnInitialize() {
        ApplyButtonSetup();
        _mapTypesCallbackId = BeatLeader::MapTypesManagerNS::Instance.AddCallback([this]() {
            ApplyMapTypes();
        }, true);
        ApplyBeatmapState();

        if (auto* captorClanView = GetCaptorClanView()) {
            CaptorClanUI::Attach(captorClanView);
        }
    }

    void LeaderboardInfoPanel::OnDispose() {
        if (_mapTypesCallbackId != 0) {
            BeatLeader::MapTypesManagerNS::Instance.RemoveCallback(_mapTypesCallbackId);
            _mapTypesCallbackId = 0;
        }
    }

    StringW LeaderboardInfoPanel::GetContent() {
        return StringW(R"(
            <horizontal pref-height="6" spacing="1" size-delta-x="87">
                <horizontal ignore-layout="true" horizontal-fit="PreferredSize" spacing="1">
                    <macro.as-host host="_mapStatus">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>

                    <macro.as-host host="_mapTypePanel0">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>

                    <macro.as-host host="_mapTypePanel1">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>

                    <macro.as-host host="_mapTypePanel2">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>

                    <macro.as-host host="_captorClan">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>
                </horizontal>

                <horizontal spacing="1" ignore-layout="true" child-expand-width="false" child-control-width="false" child-align="MiddleRight">
                    <macro.as-host host="_websiteButton">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>
                    <macro.as-host host="_settingsButton">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>
                </horizontal>
            </horizontal>
        )");
    }

    void LeaderboardInfoPanel::SetActive(bool value) {
        LocalComponent()->SetRootActive(value);
    }

    void LeaderboardInfoPanel::SetCallbacks(std::function<void()> websiteCallback, std::function<void()> settingsCallback) {
        _websiteCallback = std::move(websiteCallback);
        _settingsCallback = std::move(settingsCallback);
        ApplyButtonSetup();
    }

    void LeaderboardInfoPanel::SetMapStatusHoverCallback(std::function<void(UnityEngine::Vector3 const&, bool, float)> callback) {
        if (auto* mapStatus = GetMapStatusView()) {
            mapStatus->SetHoverCallback(std::move(callback));
        }
    }

    void LeaderboardInfoPanel::ClearBeatmapData() {
        _difficulty.reset();
        ApplyBeatmapState();
    }

    void LeaderboardInfoPanel::SetMapDifficulty(Difficulty const& difficulty) {
        _difficulty = difficulty;
        ApplyBeatmapState();
    }

    void LeaderboardInfoPanel::RefreshMapStatus() {
        if (!_difficulty.has_value()) {
            return;
        }

        if (auto* mapStatus = GetMapStatusView()) {
            mapStatus->RefreshVisuals();
        }
    }

    HeaderButton* LeaderboardInfoPanel::GetWebsiteButton() {
        if (!LocalComponent() || !LocalComponent()->_websiteButton) {
            return nullptr;
        }
        return reinterpret_cast<HeaderButton*>(LocalComponent()->_websiteButton->nativeComponent);
    }

    HeaderButton* LeaderboardInfoPanel::GetSettingsButton() {
        if (!LocalComponent() || !LocalComponent()->_settingsButton) {
            return nullptr;
        }
        return reinterpret_cast<HeaderButton*>(LocalComponent()->_settingsButton->nativeComponent);
    }

    MapStatus* LeaderboardInfoPanel::GetMapStatusView() {
        if (!LocalComponent() || !LocalComponent()->_mapStatus) {
            return nullptr;
        }
        return reinterpret_cast<MapStatus*>(LocalComponent()->_mapStatus->nativeComponent);
    }

    MapTypePanel* LeaderboardInfoPanel::GetMapTypePanel(int index) {
        if (!LocalComponent()) {
            return nullptr;
        }

        switch (index) {
            case 0:
                return LocalComponent()->_mapTypePanel0 ? reinterpret_cast<MapTypePanel*>(LocalComponent()->_mapTypePanel0->nativeComponent) : nullptr;
            case 1:
                return LocalComponent()->_mapTypePanel1 ? reinterpret_cast<MapTypePanel*>(LocalComponent()->_mapTypePanel1->nativeComponent) : nullptr;
            case 2:
                return LocalComponent()->_mapTypePanel2 ? reinterpret_cast<MapTypePanel*>(LocalComponent()->_mapTypePanel2->nativeComponent) : nullptr;
            default:
                return nullptr;
        }
    }

    CaptorClanView* LeaderboardInfoPanel::GetCaptorClanView() {
        if (!LocalComponent() || !LocalComponent()->_captorClan) {
            return nullptr;
        }
        return reinterpret_cast<CaptorClanView*>(LocalComponent()->_captorClan->nativeComponent);
    }

    void LeaderboardInfoPanel::ApplyButtonSetup() {
        if (auto* websiteButton = GetWebsiteButton()) {
            websiteButton->Setup(BundleLoader::bundle->profileIcon);
            websiteButton->SetOnClick(_websiteCallback);
        }

        if (auto* settingsButton = GetSettingsButton()) {
            settingsButton->Setup(BundleLoader::bundle->settingsIcon);
            settingsButton->SetOnClick(_settingsCallback);
        }
    }

    void LeaderboardInfoPanel::ApplyBeatmapState() {
        if (auto* mapStatus = GetMapStatusView()) {
            if (_difficulty.has_value()) {
                mapStatus->SetActive(true);
                mapStatus->SetValues(_difficulty->status, *_difficulty);
            } else {
                mapStatus->SetActive(false);
            }
        }

        if (_difficulty.has_value()) {
            CaptorClanUI::setClan(_difficulty->clanStatus);
        } else {
            CaptorClanUI::setClan(ClanRankingStatus());
        }

        ApplyMapTypes();
    }

    void LeaderboardInfoPanel::ApplyMapTypes() {
        for (int i = 0; i < 3; ++i) {
            if (auto* mapTypePanel = GetMapTypePanel(i)) {
                mapTypePanel->SetActive(false);
            }
        }

        if (!_difficulty.has_value() || !BeatLeader::MapTypesManagerNS::Instance.IsLoaded()) {
            return;
        }

        auto const& mapTypes = BeatLeader::MapTypesManagerNS::Instance.GetMapTypes();
        auto typeMask = _difficulty->type;
        size_t typeIndex = 0;

        for (int panelIndex = 0; panelIndex < 3; ++panelIndex) {
            auto* mapTypePanel = GetMapTypePanel(panelIndex);
            if (!mapTypePanel) {
                continue;
            }

            bool matched = false;
            for (; typeIndex < mapTypes.size(); ++typeIndex) {
                auto const& typeDescription = mapTypes[typeIndex];
                if (typeDescription.id > 0 && (typeMask & typeDescription.id) == typeDescription.id) {
                    mapTypePanel->SetActive(true);
                    mapTypePanel->SetValues(typeDescription);
                    ++typeIndex;
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                mapTypePanel->SetActive(false);
            }
        }
    }
}
