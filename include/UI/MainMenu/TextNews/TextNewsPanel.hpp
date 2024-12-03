#pragma once

#include "UI/MainMenu/TextNews/AbstractNewsPanel.hpp"
#include "UI/MainMenu/TextNews/TextNewsPostPanel.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "Models/NewsPost.hpp"
#include "Models/Paged.hpp"
#include <vector>

namespace BeatLeader {

class TextNewsPanel : public AbstractNewsPanel {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void ShowNews(Paged<NewsPost> news);
    void ShowError();

private:
    void PresentList(std::vector<NewsPost> const& items);
    void DisposeList();
};

} 