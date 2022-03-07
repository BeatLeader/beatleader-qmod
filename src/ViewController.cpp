#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"

#include "Utils/WebUtils.hpp"
#include "API/PlayerController.hpp"
#include "main.hpp"

#include <string>

using namespace QuestUI;
using namespace std;

UnityEngine::UI::Button* logoutButton;
HMUI::InputFieldView* loginField;
HMUI::InputFieldView* passwordField;
UnityEngine::UI::Button* loginButton;
UnityEngine::UI::Button* signupButton;
TMPro::TextMeshProUGUI* label1;
TMPro::TextMeshProUGUI* label2;
TMPro::TextMeshProUGUI* errorDescriptionLabel;

StringW login = "";
StringW password = "";

string errorDescription = "";

void UpdateUI(string userID) {
    if (userID.length() > 0) {
        label2->SetText(userID);
        label2->get_gameObject()->SetActive(true);
        label1->get_gameObject()->SetActive(true);
        logoutButton->get_gameObject()->SetActive(true);

        loginField->get_gameObject()->SetActive(false);
        passwordField->get_gameObject()->SetActive(false);
        loginButton->get_gameObject()->SetActive(false);
        signupButton->get_gameObject()->SetActive(false);
    } else {
        label2->get_gameObject()->SetActive(false);
        label1->get_gameObject()->SetActive(false);
        logoutButton->get_gameObject()->SetActive(false);

        loginField->get_gameObject()->SetActive(true);
        passwordField->get_gameObject()->SetActive(true);
        loginButton->get_gameObject()->SetActive(true);
        signupButton->get_gameObject()->SetActive(true);
    }

    errorDescriptionLabel->SetText(errorDescription);
    if (errorDescription.length() > 0) {
        errorDescriptionLabel->get_gameObject()->SetActive(true);
    } else {
        errorDescriptionLabel->get_gameObject()->SetActive(false);
    }
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    string userID = PlayerController::currentPlayer != NULL ? PlayerController::currentPlayer->id : "";

    if (firstActivation) {
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        label1 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Your id:", false);
        label2 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), userID, false);
        logoutButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Logout", [](){
            if (!PlayerController::LogOut()) {
                errorDescription = PlayerController::lastErrorDescription;
            } else {
                errorDescription = "";
                UpdateUI("");
            }
        });
        loginField = ::QuestUI::BeatSaberUI::CreateStringSetting(container->get_transform(), "Login", "", [](StringW value) {
            login = value;
        });
        login = PlayerController::platformPlayer != NULL ? PlayerController::platformPlayer->name : "";
        loginField->SetText(login);
        passwordField = ::QuestUI::BeatSaberUI::CreateStringSetting(container->get_transform(), "Password", "", [](StringW value) {
            password = value;
        });
        loginButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Log in", []() {
            string userID = PlayerController::LogIn((string)login, (string)password);
            if (userID.length() == 0) {
                errorDescription = PlayerController::lastErrorDescription;
            } else {
                errorDescription = "";
                login = "";
                loginField->SetText("");
                password = "";
                passwordField->SetText("");
            }
            UpdateUI(userID);
        });
        signupButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Sign up", []() {
            string userID = PlayerController::SignUp((string)login, (string)password);
            if (userID.length() == 0) {
                errorDescription = PlayerController::lastErrorDescription;
            } else {
                errorDescription = "";
                login = "";
                loginField->SetText("");
                password = "";
                passwordField->SetText("");
            }
            UpdateUI(userID);
        });
        errorDescriptionLabel = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), "", false);
    }

    UpdateUI(userID);
}