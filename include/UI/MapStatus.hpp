#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "shared/Models/Difficulty.hpp"

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/Vector3.hpp"

#include <functional>
#include <string>

DECLARE_CLASS_CUSTOM(BeatLeader, MapStatusComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, background);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, statusText);
};

namespace BeatLeader {

class MapStatus : public ReeUIComponentV2<MapStatusComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void SetActive(bool value);
    void SetValues(int rankedStatus, Difficulty const& difficulty);
    void RefreshVisuals();
    void SetHoverCallback(std::function<void(UnityEngine::Vector3 const&, bool, float)> callback);

private:
    void UpdateVisuals();
    static std::string GetRankedStatusText(int rankedStatus);

    std::function<void(UnityEngine::Vector3 const&, bool, float)> _hoverCallback;
    Difficulty _difficulty {};
    int _rankedStatus = -1;
    bool _hasDifficulty = false;
};

}
