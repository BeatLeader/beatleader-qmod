#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"

#include "Utils/WebUtils.hpp"
#include "include/main.hpp"
#include <string>

using namespace QuestUI;
using namespace std;

UnityEngine::UI::Button* logoutButton;
UnityEngine::UI::Button* signinButton;
UnityEngine::UI::Button* tryButton;
TMPro::TextMeshProUGUI* label1;
TMPro::TextMeshProUGUI* label2;

string TryAuth() {
    string userID = "";
    WebUtils::Get("https://beatleader.azurewebsites.net/user/id", userID);
    if (userID.length() == 0) {
        string code = (string)UnityEngine::GUIUtility::get_systemCopyBuffer();
        if (code.length() == 144) {
            string result = "";
            long statusCode = WebUtils::Get("https://beatleader.azurewebsites.net/signinoculus?token=" + code, result);
            if (statusCode == 200) {
                WebUtils::Get("https://beatleader.azurewebsites.net/user/id", userID);
            } else {
                getLogger().error("BLYAT %s", ("signin error" + to_string(statusCode)).c_str());
            }
        }
    }
    return userID;
}

void UpdateUI(string userID) {
    if (userID.length() > 0) {
        label2->SetText(userID);
        label2->set_hideFlags(UnityEngine::HideFlags::_get_None());
        label1->set_hideFlags(UnityEngine::HideFlags::_get_None());
        logoutButton->set_hideFlags(UnityEngine::HideFlags::_get_None());
        signinButton->set_hideFlags(UnityEngine::HideFlags::_get_HideInHierarchy());
        tryButton->set_hideFlags(UnityEngine::HideFlags::_get_HideInHierarchy());
    } else {
        label2->set_hideFlags(UnityEngine::HideFlags::_get_HideInHierarchy());
        label1->set_hideFlags(UnityEngine::HideFlags::_get_HideInHierarchy());
        logoutButton->set_hideFlags(UnityEngine::HideFlags::_get_HideInHierarchy());
        signinButton->set_hideFlags(UnityEngine::HideFlags::_get_None());
        tryButton->set_hideFlags(UnityEngine::HideFlags::_get_None());
    }
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    string userID = TryAuth();

    if (firstActivation) {
        self->get_gameObject()->AddComponent<HMUI::Touchable*>();
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        label1 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), "Your id:", false);
        label2 = ::QuestUI::BeatSaberUI::CreateText(container->get_transform(), userID, false);
        logoutButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Logout", [](){
            string empty = "";
            WebUtils::Get("https://beatleader.azurewebsites.net/signout", empty);
            UpdateUI("");
        });
        signinButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Signin", [](){
            UnityEngine::Application::OpenURL("https://auth.oculus.com/sso/?redirect_uri=https%3A%2F%2Fagitated-ptolemy-7d772c.netlify.app%2Fsignin%2Foculus&organization_id=702913270869417");
        });
        tryButton = ::QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "I've copied code", [](){
            UpdateUI(TryAuth());
        });
    }

    UpdateUI(userID);
}