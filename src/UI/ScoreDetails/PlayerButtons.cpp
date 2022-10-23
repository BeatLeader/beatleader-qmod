#include "include/UI/ScoreDetails/PlayerButtons.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "include/UI/EmojiSupport.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/API/PlayerController.hpp"

#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/Range.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ModConfig.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/Application.hpp"

#include "HMUI/ImageView.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static UnityEngine::Color SelectedColor = UnityEngine::Color(0.0f, 0.4f, 1.0f, 1.0f);
static UnityEngine::Color BackgroundColor = UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.8f);

static UnityEngine::Color TwitterColor = UnityEngine::Color(0.1f, 0.6f, 1.0f, 1.0f);
static UnityEngine::Color TwitchColor = UnityEngine::Color(0.5f, 0.3f, 1.0f, 1.0f);
static UnityEngine::Color YoutubeColor = UnityEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);

static Player player;
static Player currentPlayer;
static function<void(Player)> playerCallback;

void BeatLeader::PlayerButtons::Setup(HMUI::ModalView *modal, function<void(Player)> const &callback) noexcept {
    auto modalTransform = modal->get_transform();
    playerCallback = callback;
    auto captureSelf = this;

    leftBackground = UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(-0.5, 30), UnityEngine::Vector2(40, 40));
    rightBackground = UIUtils::CreateRoundRectImage(modalTransform, UnityEngine::Vector2(0.5, 30), UnityEngine::Vector2(40, 40));
    
    leftMaterial = Object::Instantiate(BundleLoader::bundle->miniProfileBackgroundMaterial);
    leftBackground->set_material(leftMaterial);

    rightMaterial = Object::Instantiate(BundleLoader::bundle->miniProfileBackgroundMaterial);
    rightBackground->set_material(rightMaterial);

    leftBackground->set_color(BackgroundColor);
    rightBackground->set_color(BackgroundColor);
    
    auto leftTransform = leftBackground->get_transform();
    auto rightTransform = rightBackground->get_transform();

    friendsButton = MiniProfileButton("Friends management", SelectedColor, true, ::QuestUI::BeatSaberUI::CreateClickableImage(
            leftTransform, 
            BundleLoader::bundle->friendsIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->toggleFriend();
    }));
    friendsButton.RegisterCallback();

    incognitoButton = MiniProfileButton("Hide player", SelectedColor, true, ::QuestUI::BeatSaberUI::CreateClickableImage(
            leftTransform, 
            BundleLoader::bundle->incognitoIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->toggleBlacklist();
    }));
    incognitoButton.RegisterCallback();

    linkButton = MiniProfileButton("Open profile", SelectedColor, true, ::QuestUI::BeatSaberUI::CreateClickableImage(
            leftTransform, 
            BundleLoader::bundle->profileIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->openProfile();
    }));
    linkButton.RegisterCallback();

    twitterButton = MiniProfileButton("Twitter", TwitterColor, false, ::QuestUI::BeatSaberUI::CreateClickableImage(
            rightTransform, 
            BundleLoader::bundle->twitterIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->openSocial("Twitter");
    }));
    twitterButton.RegisterCallback();

    twitchButton = MiniProfileButton("Twitch", TwitchColor, false, ::QuestUI::BeatSaberUI::CreateClickableImage(
            rightTransform, 
            BundleLoader::bundle->twitchIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->openSocial("Twitch");
    }));
    twitchButton.RegisterCallback();

    youtubeButton = MiniProfileButton("YouTube", YoutubeColor, false, ::QuestUI::BeatSaberUI::CreateClickableImage(
            rightTransform, 
            BundleLoader::bundle->youtubeIcon, 
            {0, 0}, 
            {4, 4}, [captureSelf](){
        captureSelf->openSocial("YouTube");
    }));
    youtubeButton.RegisterCallback();

    UpdateLayout();
}

void BeatLeader::PlayerButtons::setScore(Score score) {
    player = score.player;
    if (PlayerController::currentPlayer && !PlayerController::currentPlayer->name.empty()) {
        currentPlayer = PlayerController::currentPlayer.value();
    }

    updateFriendButton();
    updateSocialButtons();
    updateIncognitoButton();
}

void BeatLeader::PlayerButtons::toggleFriend() const {
    friendsButton.setState(MiniProfileButtonState::NonInteractable);
    auto captureSelf = this;
    if (!PlayerController::IsFriend(player)) {
        WebUtils::RequestAsync(WebUtils::API_URL + "user/friend?playerId=" + player.id, "POST", 60, [captureSelf](long status, string response) {
            captureSelf->friendsButton.setHint("Remove friend");
            captureSelf->friendsButton.setState(MiniProfileButtonState::InteractableGlowing);
            PlayerController::currentPlayer->friends.push_back(player.id);
        });
    } else {
        WebUtils::RequestAsync(WebUtils::API_URL + "user/friend?playerId=" + player.id, "DELETE", 60, [captureSelf](long status, string response) {
            captureSelf->friendsButton.setHint("Add friend");
            captureSelf->friendsButton.setState(MiniProfileButtonState::InteractableFaded);
            auto iterator = PlayerController::currentPlayer->friends.begin();

            for (auto it = PlayerController::currentPlayer->friends.begin(); it != PlayerController::currentPlayer->friends.end(); it++) {
                if (*it == player.id) {
                    PlayerController::currentPlayer->friends.erase(it);
                    break;
                }
            }
        });
    }
}

void BeatLeader::PlayerButtons::toggleBlacklist() const {
    incognitoButton.setState(MiniProfileButtonState::NonInteractable);

    if (!PlayerController::IsIncognito(player)) {
        incognitoButton.setHint("Show player");
        incognitoButton.setState(MiniProfileButtonState::InteractableGlowing);
        PlayerController::SetIsIncognito(player, true);
    } else {
        incognitoButton.setHint("Hide player");
        incognitoButton.setState(MiniProfileButtonState::InteractableFaded);
        PlayerController::SetIsIncognito(player, false);
    }
    playerCallback(player);
}

void BeatLeader::PlayerButtons::updateFriendButton() const {
    if (player.id == currentPlayer.id) {
        friendsButton.setHint("Friends management");
        friendsButton.setState(MiniProfileButtonState::NonInteractable);

    } else {
        bool isFriend = PlayerController::IsFriend(player);

        if (isFriend) {
            friendsButton.setHint("Remove friend");
            friendsButton.setState(MiniProfileButtonState::InteractableGlowing);
        } else {
            friendsButton.setHint("Add friend");
            friendsButton.setState(MiniProfileButtonState::InteractableFaded);
        }
    }
}

void BeatLeader::PlayerButtons::updateSocialButtons() const {
    if (!PlayerController::IsPatron(player)) {
        twitterButton.setState(MiniProfileButtonState::Hidden);
        twitchButton.setState(MiniProfileButtonState::Hidden);
        youtubeButton.setState(MiniProfileButtonState::Hidden);

        rightBackground->get_gameObject()->SetActive(false);
    } else {
        twitterButton.setState(MiniProfileButtonState::NonInteractable);
        twitchButton.setState(MiniProfileButtonState::NonInteractable);
        youtubeButton.setState(MiniProfileButtonState::NonInteractable);

        rightBackground->get_gameObject()->SetActive(false);

        for (size_t i = 0; i < player.socials.size(); i++)
        {
            if (player.socials[i].service == "Twitter") {
                twitterButton.setState(MiniProfileButtonState::InteractableGlowing);
            }
            if (player.socials[i].service == "Twitch") {
                twitchButton.setState(MiniProfileButtonState::InteractableGlowing);
            }
            if (player.socials[i].service == "YouTube") {
                youtubeButton.setState(MiniProfileButtonState::InteractableGlowing);
            }
        }
    }
}

void BeatLeader::PlayerButtons::updateIncognitoButton() const {
    if (!PlayerController::IsIncognito(player)) {
        incognitoButton.setState(MiniProfileButtonState::InteractableFaded);
    } else {
        incognitoButton.setState(MiniProfileButtonState::InteractableGlowing);
    }
}

void BeatLeader::PlayerButtons::openSocial(string name) const {
    Social social;
    for (size_t i = 0; i < player.socials.size(); i++)
    {
        if (player.socials[i].service == name) {
            social = player.socials[i];
            break;
        }
    }

    UnityEngine::Application::OpenURL(social.link);
}

void BeatLeader::PlayerButtons::openProfile() const {
    string url = WebUtils::WEB_URL + "u/" + player.id;
    UnityEngine::Application::OpenURL(url);
}

const float Deg2Rad = 0.017f;
const float Radius = 16.8f;
const float StepRadians = Deg2Rad * 18;
const float OffsetRadians = Deg2Rad * 90;
const float BackgroundThickness = 6.1f / 40.0f;

static void UpdateLayoutInternal(
    Transform* parent, float offsetRadians, float stepRadians,
    float* fromRadians, float* toRadians
) {
    int activeCount = 0;
    for (int i = 0; i < parent->get_childCount(); i++) {
        auto child = parent->GetChild(i);
        if (child->get_gameObject()->get_activeSelf()) activeCount += 1;
    }

    float fromRadiansValue = offsetRadians - (activeCount - 1) * stepRadians / 2;
    *fromRadians = fromRadiansValue;
    *toRadians = fromRadiansValue + stepRadians * (activeCount - 1);

    if (activeCount <= 0) {
        parent->get_gameObject()->SetActive(false);
        return;
    }

    parent->get_gameObject()->SetActive(true);

    float angleRadians = 3.1415f / 2 + fromRadiansValue;
    for (int i = 0; i < parent->get_childCount(); i++) {
        auto child = parent->GetChild(i);
        if (!child->get_gameObject()->get_activeSelf()) continue;
        child->set_localPosition(Vector3(
            cos(angleRadians) * Radius,
            sin(angleRadians) * Radius,
            0
        ));
        angleRadians += stepRadians;
    }
}

void BeatLeader::PlayerButtons::UpdateLayout() {
    float leftA, leftB, rightA, rightB;

    UpdateLayoutInternal(leftBackground->get_transform(), OffsetRadians, StepRadians, &leftA, &leftB);
    UpdateLayoutInternal(rightBackground->get_transform(), -OffsetRadians, -StepRadians, &rightA, &rightB);
    UpdateBackground(leftA, leftB, rightB, rightA);
}

static UnityEngine::Vector4 GetParamsVector(float fromRadians, float toRadians, float thickness) {
    return UnityEngine::Vector4(fromRadians, toRadians, thickness, 1.0f);
}

void BeatLeader::PlayerButtons::UpdateBackground(float leftFrom, float leftTo, float rightFrom, float rightTo) {
    int ParamsPropertyId = Shader::PropertyToID("_Params");

    leftMaterial->SetVector(ParamsPropertyId, GetParamsVector(leftFrom, leftTo, BackgroundThickness));
    rightMaterial->SetVector(ParamsPropertyId, GetParamsVector(rightFrom, rightTo, BackgroundThickness));
}
