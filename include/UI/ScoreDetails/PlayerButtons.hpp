#pragma once

#include "HMUI/ModalView.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include "include/Models/Score.hpp"
#include "include/Models/Player.hpp"
#include "include/UI/ScoreDetails/MiniProfileButton.hpp"

namespace BeatLeader {
    class PlayerButtons {
        public:
            PlayerButtons() = default;
            void Setup(HMUI::ModalView* modal, function<void(Player)> const &callback) noexcept;

            HMUI::ImageView* leftBackground;
            HMUI::ImageView* rightBackground;

            UnityEngine::Material* leftMaterial;
            UnityEngine::Material* rightMaterial;

            MiniProfileButton friendsButton;
            MiniProfileButton incognitoButton;
            MiniProfileButton linkButton;

            MiniProfileButton twitterButton;
            MiniProfileButton twitchButton;
            MiniProfileButton youtubeButton;
            
            void setScore(Score score);

            void openSocial(string name) const;
            void openProfile() const;

            void updateSocialButtons() const;
            void updateFriendButton() const;
            void updateIncognitoButton() const;

            void toggleFriend() const;
            void toggleBlacklist() const;

            void UpdateLayout();
            void UpdateBackground(float leftFrom, float leftTo, float rightFrom, float rightTo);
    };
}