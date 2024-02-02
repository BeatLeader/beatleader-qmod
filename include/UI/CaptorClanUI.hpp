#pragma once

#include "include/Models/Clan.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "TMPro/TextMeshProUGUI.hpp"

namespace CaptorClanUI {
    extern bool showClanRanking;
    extern function<void()> showClanRankingCallback; 

    void initCaptorClan(UnityEngine::GameObject* header, TMPro::TextMeshProUGUI* headerPanelText);
    void setClan(ClanRankingStatus clanStatus);
    void setActive(bool active);
}