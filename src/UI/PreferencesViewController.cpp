#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "System/Action_2.hpp"

#include "HMUI/DropdownWithTableView.hpp"

#include "custom-types/shared/delegate.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "API/PlayerController.hpp"
#include "include/Core/ReplayPlayer.hpp"

#include "include/UI/PreferencesViewController.hpp"
#include "include/UI/LevelInfoUI.hpp"
#include "include/UI/LogoAnimation.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "main.hpp"

#include <string>

using namespace QuestUI;
using namespace std;

DEFINE_TYPE(BeatLeader, PreferencesViewController);

UnityEngine::UI::Button* logoutButton;
HMUI::InputFieldView* loginField;
HMUI::InputFieldView* passwordField;
UnityEngine::UI::Button* loginButton;
UnityEngine::UI::Button* signupButton;
TMPro::TextMeshProUGUI* name;
TMPro::TextMeshProUGUI* label3;
TMPro::TextMeshProUGUI* errorDescriptionLabel;

HMUI::SimpleTextDropdown* serverDropdown;
UnityEngine::UI::Toggle* saveToggle;
UnityEngine::UI::Toggle* showReplaySettingsToggle;

BeatLeader::LogoAnimation* spinner = NULL;

string login;
string password;

string errorDescription = "";

void UpdateUI(optional<Player> player) {
    spinner->imageView->get_gameObject()->SetActive(false);
    spinner->SetAnimating(false);

    if (player != nullopt) {
        name->SetText(player->name + ", hi!");
        name->get_gameObject()->SetActive(true);
        label3->get_gameObject()->SetActive(false);
        logoutButton->get_gameObject()->SetActive(true);

        loginField->get_gameObject()->SetActive(false);
        passwordField->get_gameObject()->SetActive(false);
        loginButton->get_gameObject()->SetActive(false);
        signupButton->get_gameObject()->SetActive(false);
    } else {
        name->get_gameObject()->SetActive(false);
        label3->get_gameObject()->SetActive(true);
        logoutButton->get_gameObject()->SetActive(false);

        loginField->get_gameObject()->SetActive(true);
        passwordField->get_gameObject()->SetActive(true);
        loginButton->get_gameObject()->SetActive(true);
        signupButton->get_gameObject()->SetActive(true);

        loginField->set_interactable(true);
        passwordField->set_interactable(true);
        loginButton->set_interactable(true);
        signupButton->set_interactable(true);
    }

    errorDescriptionLabel->SetText(errorDescription);
    if (errorDescription.length() > 0) {
        errorDescriptionLabel->get_gameObject()->SetActive(true);
    } else {
        errorDescriptionLabel->get_gameObject()->SetActive(false);
    }
}

void showLoading() {
    spinner->imageView->get_gameObject()->SetActive(true);
    spinner->SetAnimating(true);

    loginField->set_interactable(false);
    passwordField->set_interactable(false);
    loginButton->set_interactable(false);
    signupButton->set_interactable(false);
}

void BeatLeader::PreferencesViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
    errorDescription = "";
}

void BeatLeader::PreferencesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        this->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(this->get_transform());

        auto containerTransform = container->get_transform();

        auto spinnerImage = ::QuestUI::BeatSaberUI::CreateImage(this->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, {0, 20}, {20, 20});
        spinner = this->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
        spinner->Init(spinnerImage);
        spinnerImage->get_gameObject()->SetActive(false);

        name = ::QuestUI::BeatSaberUI::CreateText(containerTransform, "", false);
        EmojiSupport::AddSupport(name);

        logoutButton = ::QuestUI::BeatSaberUI::CreateUIButton(containerTransform, "Logout", [](){
            PlayerController::LogOut();
        });

        loginField = ::QuestUI::BeatSaberUI::CreateStringSetting(containerTransform, "Login", "", [](StringW value) {
            login = (string) value;
        });
        
        passwordField = ::QuestUI::BeatSaberUI::CreateStringSetting(containerTransform, "Password", "", [](StringW value) {
            password = (string) value;
        });

        loginButton = ::QuestUI::BeatSaberUI::CreateUIButton(containerTransform, "Log in", [spinnerImage]() {
            if (login.empty() || password.empty()) {
                errorDescription = "Enter a username and/or password!";
                UpdateUI(nullopt);
                return;
            }
            showLoading();
            PlayerController::LogIn(login, password, [](std::optional<Player> const& player, string error) {
                QuestUI::MainThreadScheduler::Schedule([player, error] {
                if (player == nullopt) {
                    errorDescription = error;
                } else {
                    errorDescription = "";
                    login = "";
                    loginField->SetText("");
                    password = "";
                    passwordField->SetText("");
                }
                UpdateUI(player);
                });
            });
        });
        signupButton = ::QuestUI::BeatSaberUI::CreateUIButton(containerTransform, "Sign up", []() {
            if (login.empty() || password.empty()) {
                errorDescription = "Enter a username and/or password!";
                UpdateUI(nullopt);
                return;
            }
            showLoading();
            PlayerController::SignUp((string)login, (string)password, [](std::optional<Player> const& player, string error) {
                QuestUI::MainThreadScheduler::Schedule([player, error] {
                if (player == nullopt) {
                    errorDescription = error;
                } else {
                    errorDescription = "";
                    login = "";
                    loginField->SetText("");
                    password = "";
                    passwordField->SetText("");
                }
                UpdateUI(player);
                });
            });
        });

        auto captureSelf = this;
        PlayerController::playerChanged.emplace_back([captureSelf](std::optional<Player> const& updated) {
            if (!captureSelf->isActivated) return;
            QuestUI::MainThreadScheduler::Schedule([updated] {
                UpdateUI(updated);
            });
        });

        saveToggle = AddConfigValueToggle(containerTransform, getModConfig().SaveLocalReplays);
        auto dropdown = AddConfigValueDropdownEnum(containerTransform, getModConfig().StarValueToShow, {
            "Overall",
            "Tech",
            "Acc",
            "Pass"
        });
        // After switching the setting we need to manually call refresh, because StandardLevelDetailView::RefreshContent is not called again,
        // if the same map, that was selected before changing the setting, is selected again before selecting any other map. 
        // This results in setLabels not being called again and the stars of the old setting are displayed, which is why we call it manually here after selecting an option
        dropdown->add_didSelectCellWithIdxEvent(custom_types::MakeDelegate<System::Action_2<HMUI::DropdownWithTableView*, int>*>((function<void(HMUI::DropdownWithTableView*, int)>)[](auto throwaway1, auto throwaway2){
            LevelInfoUI::refreshLabelsDiff();
        }));
        if (ReplayInstalled()) {
            showReplaySettingsToggle = AddConfigValueToggle(containerTransform, getModConfig().ShowReplaySettings);
        }
        errorDescriptionLabel = ::QuestUI::BeatSaberUI::CreateText(containerTransform, "", false);
        label3 = ::QuestUI::BeatSaberUI::CreateText(containerTransform, "To sign up, enter your login information.\nTo log in, enter your existing account's login information.\nYour account is temporary until at least one score has been posted!\nYou can change your profile picture on the website.", false);
    }

    UpdateUI(PlayerController::currentPlayer);
}