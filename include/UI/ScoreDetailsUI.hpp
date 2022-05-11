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


namespace BeatLeader {
    class ModalPopup {
        public:
            HMUI::ModalView* modal;

            HMUI::ImageView* playerAvatar;
            UnityEngine::UI::VerticalLayoutGroup* list;

            UnityEngine::UI::HorizontalLayoutGroup* header;
            TMPro::TextMeshProUGUI* rank;
            TMPro::TextMeshProUGUI* name;
            TMPro::TextMeshProUGUI* pp;

            TMPro::TextMeshProUGUI* datePlayed;
            
            UnityEngine::UI::HorizontalLayoutGroup* header2;
            TMPro::TextMeshProUGUI* modifiedScore;
            TMPro::TextMeshProUGUI* accuracy;
            TMPro::TextMeshProUGUI* scorePp;

            TMPro::TextMeshProUGUI* scoreDetails;
            
            UnityEngine::UI::Button* closeButton;
            
            void setScore(const Score& score);
    };
    void initModalPopup(ModalPopup** modalUI, UnityEngine::Transform* parent);
}