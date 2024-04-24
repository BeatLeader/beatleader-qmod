#pragma once

#include "include/Models/Clan.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "TMPro/TextMeshProUGUI.hpp"

namespace CaptorClanUI {
    extern bool showClanRanking;
    extern function<void()> showClanRankingCallback; 

    void initCaptorClan(UnityEngine::GameObject* header, TMPro::TextMeshProUGUI* headerPanelText);
    void setClan(ClanRankingStatus clanStatus);
    void setActive(bool active);
    void Reset();
}