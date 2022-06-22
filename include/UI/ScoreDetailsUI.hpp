#pragma once

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/register.hpp"
#include "modloader/shared/modloader.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "HMUI/ModalView.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "GlobalNamespace/ScoreFormatter.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "include/Models/Score.hpp"
#include "include/UI/PlayerAvatar.hpp"

namespace BeatLeader {
    class ScoreDetailsPopup {
        public:
            HMUI::ModalView* modal;

            BeatLeader::PlayerAvatar* playerAvatar;

            TMPro::TextMeshProUGUI* rank;
            TMPro::TextMeshProUGUI* name;
            TMPro::TextMeshProUGUI* pp;

            TMPro::TextMeshProUGUI* datePlayed;
            
            TMPro::TextMeshProUGUI* modifiedScore;
            TMPro::TextMeshProUGUI* accuracy;
            TMPro::TextMeshProUGUI* scorePp;

            TMPro::TextMeshProUGUI* scoreDetails;
            TMPro::TextMeshProUGUI* sponsorMessage;
            
            UnityEngine::UI::Button* closeButton;
            
            void setScore(const Score& score);
    };
    void initScoreDetailsPopup(ScoreDetailsPopup** modalUI, UnityEngine::Transform* parent);
}