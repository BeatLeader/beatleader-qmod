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

enum MapType {
    notSelected = 1 << -1,
    acc = 1 << 0,
    tech = 1 << 1,
    midspeed = 1 << 2,
    speed = 1 << 3
};

namespace BeatLeader {
    class RankVotingPopup {
        public:
            HMUI::ModalView* modal;

            TMPro::TextMeshProUGUI* header1;
            UnityEngine::UI::Button* yesButton;
            UnityEngine::UI::Button* noButton;

            TMPro::TextMeshProUGUI* header2;
            QuestUI::SliderSetting* starSlider;
            TMPro::TextMeshProUGUI* subheader2;

            UnityEngine::UI::Button* accButton;
            UnityEngine::UI::Button* techButton;
            UnityEngine::UI::Button* midspeedButton;
            UnityEngine::UI::Button* speedButton;
            
            UnityEngine::UI::Button* cancelButton;
            UnityEngine::UI::Button* voteButton;
            UnityEngine::UI::Button* leftButton;
            UnityEngine::UI::Button* rightButton;

            bool rankable;
            float stars;
            int type;

            void left() const;
            void right() const;
            void reset();

            void updateType(MapType mapType, UnityEngine::UI::Button* button);
    };
    void initVotingPopup(RankVotingPopup** modalUI, UnityEngine::Transform* parent, function<void(bool, bool, float, int)> const &callback);
}