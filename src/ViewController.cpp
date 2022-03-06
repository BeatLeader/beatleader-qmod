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
UnityEngine::UI::Button* signinButton;
UnityEngine::UI::Button* tryButton;
TMPro::TextMeshProUGUI* label1;
TMPro::TextMeshProUGUI* label2;
TMPro::TextMeshProUGUI* errorLabel;

string errorDescription;

void UpdateUI(string userID) {
    if (userID.length() > 0) {
        label2->SetText(il2cpp_utils::createcsstr(userID));
        label2->get_gameObject()->SetActive(true);
        label1->get_gameObject()->SetActive(true);
        logoutButton->get_gameObject()->SetActive(true);
        signinButton->get_gameObject()->SetActive(false);
        tryButton->get_gameObject()->SetActive(false);
    } else {
        label2->get_gameObject()->SetActive(false);
        label1->get_gameObject()->SetActive(false);
        logoutButton->get_gameObject()->SetActive(false);
        signinButton->get_gameObject()->SetActive(true);
        tryButton->get_gameObject()->SetActive(true);
    }

    errorLabel->SetText(il2cpp_utils::createcsstr(errorDescription));
    if (errorDescription.length() > 0) {
        errorLabel->get_gameObject()->SetActive(true);
    } else {
        errorLabel->get_gameObject()->SetActive(false);
    }
}

string TryAuth() {
    errorDescription = "";
    string userID = PlayerController::currentPlayer != NULL ? PlayerController::currentPlayer->id : "";
    if (userID.length() == 0) {
        userID = PlayerController::LogIn(to_utf8(csstrtostr(UnityEngine::GUIUtility::get_systemCopyBuffer())));
        if (userID.length() == 0) {
            errorDescription = PlayerController::lastErrorDescription;
        }
    }
    return userID;
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    string userID = TryAuth();

    if (firstActivation) {
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        label1 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Your id:", false);
        label2 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), userID, false);
        logoutButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Logout", [](){
            if (!PlayerController::LogOut()) {
                errorDescription = PlayerController::lastErrorDescription;
                UpdateUI(PlayerController::currentPlayer->id);
            } else {
                errorDescription = "";
                UpdateUI("");
            }
        });
        signinButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Signin", [](){
            UnityEngine::Application::OpenURL(il2cpp_utils::createcsstr("https://auth.oculus.com/sso/?redirect_uri=https%3A%2F%2Fagitated-ptolemy-7d772c.netlify.app%2Fsignin%2Foculus&organization_id=702913270869417"));
        });
        tryButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "I've copied code", [](){
            UpdateUI(TryAuth());
        });
        errorLabel = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), errorDescription, false);
    }

    UpdateUI(userID);
}