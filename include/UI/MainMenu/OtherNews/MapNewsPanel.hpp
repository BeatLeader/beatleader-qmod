#pragma once
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UI/MainMenu/OtherNews/FeaturedPreviewPanel.hpp"
#include "UI/MainMenu/TextNews/AbstractNewsPanel.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "Models/TrendingMapData.hpp"
#include "Models/Paged.hpp"

namespace BeatLeader {

class MapNewsPanel : public AbstractNewsPanel {
public:
    void OnInitialize() override;
    StringW GetContent() override;
    bool HasHeader() override;

    // Public methods
    void PresentList(std::vector<TrendingMapData> const& items);
    void DisposeList();
};

}  // namespace BeatLeader 