#include "HMUI/Touchable.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

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
#include "include/UI/QuestUI.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "main.hpp"

#include <string>

using namespace BSML;
using namespace std;

DEFINE_TYPE(BeatLeader, PreferencesViewController);

UnityEngine::Transform* containerTransform;

UnityEngine::UI::Button* logoutButton;
HMUI::InputFieldView* loginField;
HMUI::InputFieldView* passwordField;
UnityEngine::UI::Button* loginButton;
UnityEngine::UI::Button* signupButton;
TMPro::TextMeshProUGUI* nameField;
TMPro::TextMeshProUGUI* label3;
TMPro::TextMeshProUGUI* errorDescriptionLabel;

BSML::DropdownListSetting* serverDropdown;
BSML::DropdownListSetting* starsDropdown;
BSML::ToggleSetting* saveToggle;
BSML::ToggleSetting* showReplaySettingsToggle;
BSML::ToggleSetting* noticeboardToggle;
BeatLeader::LogoAnimation* spinner = NULL;

string login;
string password;

string errorDescription = "";

void UpdateUI(optional<Player> player) {
    spinner->imageView->get_gameObject()->SetActive(false);
    spinner->SetAnimating(false);

    if (player != nullopt) {
        nameField->SetText(player->name + ", hi!", true);
        nameField->get_gameObject()->SetActive(true);
        label3->get_gameObject()->SetActive(false);
        logoutButton->get_gameObject()->SetActive(true);

        loginField->get_gameObject()->SetActive(false);
        passwordField->get_gameObject()->SetActive(false);
        loginButton->get_gameObject()->SetActive(false);
        signupButton->get_gameObject()->SetActive(false);

        saveToggle->get_gameObject()->SetActive(true);
        starsDropdown->get_transform()->get_parent()->get_gameObject()->SetActive(true);
        if(showReplaySettingsToggle)
            showReplaySettingsToggle->get_gameObject()->SetActive(true);
    } else {
        nameField->get_gameObject()->SetActive(false);
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

        saveToggle->get_gameObject()->SetActive(false);
        starsDropdown->get_transform()->get_parent()->get_gameObject()->SetActive(false);
        if(showReplaySettingsToggle)
            showReplaySettingsToggle->get_gameObject()->SetActive(false);
    }

    errorDescriptionLabel->SetText(errorDescription, true);
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

std::vector<std::string_view> starValueOptions = {
    "Overall",
    "Tech",
    "Acc",
    "Pass"
};

std::vector<std::string_view> serverOptions = {
    ".com",
    ".net",
    ".org"
};

void BeatLeader::PreferencesViewController::DidActivate(bool firstActivation, bool addedToHeirarchy, bool screenSystemDisabling) {
    if (firstActivation) {
        // Make Touchable
        this->get_gameObject()->AddComponent<HMUI::Touchable*>();

        // Create Container
        auto* container = BSML::Lite::CreateScrollableSettingsContainer(this->get_transform());

        containerTransform = container->get_transform();

        auto spinnerImage = ::BSML::Lite::CreateImage(this->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, {0, 20}, {20, 20});
        spinner = this->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
        spinner->Init(spinnerImage);
        spinnerImage->get_gameObject()->SetActive(false);

        nameField = ::BSML::Lite::CreateText(containerTransform, "");
        EmojiSupport::AddSupport(nameField);

        logoutButton = ::BSML::Lite::CreateUIButton(containerTransform, "Logout", [](){
            PlayerController::LogOut();
            UpdateUI(nullopt);
        });

        loginField = ::BSML::Lite::CreateStringSetting(containerTransform, "Login", "", [](StringW value) {
            login = (string) value;
        });
        
        passwordField = ::BSML::Lite::CreateStringSetting(containerTransform, "Password", "", [](StringW value) {
            password = (string) value;
        });

        loginButton = ::BSML::Lite::CreateUIButton(containerTransform, "Log in", []() {
            if (login.empty() || password.empty()) {
                errorDescription = "Enter a username and/or password!";
                UpdateUI(nullopt);
                return;
            }
            showLoading();
            PlayerController::LogIn(login, password, [](std::optional<Player> const& player, string error) {
                BSML::MainThreadScheduler::Schedule([player, error] {
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
        signupButton = ::BSML::Lite::CreateUIButton(containerTransform, "Sign up", []() {
            if (login.empty() || password.empty()) {
                errorDescription = "Enter a username and/or password!";
                UpdateUI(nullopt);
                return;
            }
            showLoading();
            PlayerController::SignUp((string)login, (string)password, [](std::optional<Player> const& player, string error) {
                BSML::MainThreadScheduler::Schedule([player, error] {
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

        saveToggle = AddConfigValueToggle(containerTransform, getModConfig().SaveLocalReplays);
        starsDropdown = AddConfigValueDropdownEnum(containerTransform, getModConfig().StarValueToShow, starValueOptions);
        // // After switching the setting we need to manually call refresh, because StandardLevelDetailView::RefreshContent is not called again,
        // // if the same map, that was selected before changing the setting, is selected again before selecting any other map. 
        // // This results in setLabels not being called again and the stars of the old setting are displayed, which is why we call it manually here after selecting an option
        starsDropdown->dropdown->add_didSelectCellWithIdxEvent(custom_types::MakeDelegate<System::Action_2<UnityW<HMUI::DropdownWithTableView>, int>*>((function<void(UnityW<HMUI::DropdownWithTableView>, int)>)[](auto throwaway1, auto throwaway2){
            LevelInfoUI::refreshRatingLabels();
        }));

        serverDropdown = AddConfigValueDropdownEnum(containerTransform, getModConfig().DomainType, serverOptions);
        serverDropdown->dropdown->add_didSelectCellWithIdxEvent(custom_types::MakeDelegate<System::Action_2<UnityW<HMUI::DropdownWithTableView>, int>*>((function<void(UnityW<HMUI::DropdownWithTableView>, int)>)[](auto throwaway1, auto throwaway2){
            WebUtils::refresh_urls();
        }));
        if (ReplayInstalled()) {
            showReplaySettingsToggle = AddConfigValueToggle(containerTransform, getModConfig().ShowReplaySettings);
        }
        errorDescriptionLabel = QuestUI::CreateText(containerTransform, "");
        errorDescriptionLabel->set_color(UnityEngine::Color(1.0, 0.0, 0.0, 1));
        label3 = QuestUI::CreateText(containerTransform, "Don't have BeatLeader Quest profile? Sign up with any new login/password.\nYou can change details or merge with the Steam profile on the website.");
        label3->set_fontSize(3.0f);
        noticeboardToggle = AddConfigValueToggle(containerTransform, getModConfig().NoticeboardEnabled);
    }

    UpdateUI(PlayerController::currentPlayer);
}

void BeatLeader::PreferencesViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
    errorDescription = "";
}