#pragma once

#include "UI/MainMenu/TextNews/AbstractNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/FeaturedPreviewPanel.hpp"
#include "UI/MainMenu/OtherNews/NewsHeader.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/ViewController.hpp"
#include "Models/PlatformEvent.hpp"
#include <vector>

namespace BeatLeader {

class EventNewsPanel : public AbstractNewsPanel {
public:
    void OnInitialize() override;
    StringW GetContent() override;
    bool HasHeader() override;

private:
    void PresentList(const std::vector<PlatformEvent>& items);
    void DisposeList();
};

} 