#include "HMUI/Touchable.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "System/Action_2.hpp"

#include "HMUI/DropdownWithTableView.hpp"

#include "custom-types/shared/delegate.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "API/PlayerController.hpp"
#include "include/Managers/LeaderboardHeaderManager.hpp"
#include "include/Core/ReplayPlayer.hpp"

#include "include/UI/PreferencesViewController.hpp"
#include "include/UI/LogoAnimation.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/QuestUI.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "main.hpp"

#include <string>

using namespace BSML;
using namespace std;

DEFINE_TYPE(BeatLeader, PreferencesViewController);

namespace {
    enum class PreferencesTab {
        Account = 0,
        General = 1,
        Replays = 2
    };

    constexpr float TabSelectorY = 38.0f;
    constexpr float TabSelectorWidth = 78.0f;
    constexpr float TabSelectorHeight = 7.0f;
    constexpr float PagePreferredWidth = 88.0f;
}

UnityEngine::Transform* contentContainerTransform = nullptr;
UnityEngine::Transform* accountTabTransform = nullptr;
UnityEngine::Transform* generalTabTransform = nullptr;
UnityEngine::Transform* replaysTabTransform = nullptr;
UnityEngine::Transform* tabSelectorContainerTransform = nullptr;
QuestUI::CustomTextSegmentedControlData* tabSelector = nullptr;
int currentTab = static_cast<int>(PreferencesTab::Account);

UnityEngine::UI::Button* logoutButton = nullptr;
HMUI::InputFieldView* loginField = nullptr;
HMUI::InputFieldView* passwordField = nullptr;
UnityEngine::UI::Button* loginButton = nullptr;
UnityEngine::UI::Button* signupButton = nullptr;
TMPro::TextMeshProUGUI* nameField = nullptr;
TMPro::TextMeshProUGUI* label3 = nullptr;
TMPro::TextMeshProUGUI* errorDescriptionLabel = nullptr;

BSML::DropdownListSetting* serverDropdown = nullptr;
BSML::DropdownListSetting* starsDropdown = nullptr;
BSML::ToggleSetting* showReplaySettingsToggle = nullptr;
BSML::ToggleSetting* noticeboardToggle = nullptr;
BSML::ToggleSetting* keepReplaysToggle = nullptr;
BSML::ToggleSetting* keepFailToggle = nullptr;
BSML::ToggleSetting* keepExitToggle = nullptr;
BSML::ToggleSetting* keepPracticeToggle = nullptr;
BSML::ToggleSetting* saveEveryAttemptToggle = nullptr;
BeatLeader::LogoAnimation* spinner = nullptr;

string login;
string password;
string errorDescription = "";

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

UnityEngine::Transform* CreateAnchoredContainer(UnityEngine::Transform* parent, StringW name, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
    auto* gameObject = UnityEngine::GameObject::New_ctor(name);
    auto* rectTransform = gameObject->AddComponent<UnityEngine::RectTransform*>();
    rectTransform->SetParent(parent, false);
    rectTransform->set_anchorMin({0.5f, 0.5f});
    rectTransform->set_anchorMax({0.5f, 0.5f});
    rectTransform->set_pivot({0.5f, 0.5f});
    rectTransform->set_anchoredPosition(anchoredPosition);
    rectTransform->set_sizeDelta(sizeDelta);
    return rectTransform;
}

UnityEngine::Transform* CreateTabPage(UnityEngine::Transform* parent, StringW name) {
    auto* pageTransform = CreateAnchoredContainer(parent, name, {0.0f, 0.0f}, {PagePreferredWidth, 0.0f});
    auto* layoutGroup = pageTransform->get_gameObject()->AddComponent<UnityEngine::UI::VerticalLayoutGroup*>();
    layoutGroup->set_childControlWidth(true);
    layoutGroup->set_childControlHeight(true);
    layoutGroup->set_childForceExpandWidth(true);
    layoutGroup->set_childForceExpandHeight(false);

    auto* fitter = pageTransform->get_gameObject()->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
    fitter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::Unconstrained);
    fitter->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);

    auto* layoutElement = pageTransform->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement*>();
    layoutElement->set_preferredWidth(PagePreferredWidth);
    return pageTransform;
}

UnityEngine::Transform* GetTabTransform(PreferencesTab tab) {
    switch (tab) {
        case PreferencesTab::Account:
            return accountTabTransform;
        case PreferencesTab::General:
            return generalTabTransform;
        case PreferencesTab::Replays:
        default:
            return replaysTabTransform;
    }
}

void RefreshLayout(UnityEngine::Transform* transform) {
    if (!transform) {
        return;
    }

    auto* rectTransform = transform->GetComponent<UnityEngine::RectTransform*>();
    if (rectTransform) {
        UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(rectTransform);
    }
}

void RefreshCurrentTabLayout() {
    RefreshLayout(GetTabTransform(static_cast<PreferencesTab>(currentTab)));
    RefreshLayout(contentContainerTransform);
}

void SetSettingInteractable(BSML::ToggleSetting* setting, bool interactable) {
    if (!setting) {
        return;
    }

    auto toggles = setting->GetComponentsInChildren<UnityEngine::UI::Toggle*>();
    for (int i = 0; i < toggles.size(); ++i) {
        if (toggles[i]) {
            toggles[i]->set_interactable(interactable);
        }
    }

    auto texts = setting->GetComponentsInChildren<TMPro::TextMeshProUGUI*>();
    for (int i = 0; i < texts.size(); ++i) {
        auto* text = texts[i];
        if (!text) {
            continue;
        }

        auto color = text->get_color();
        color.a = interactable ? 1.0f : 0.5f;
        text->set_color(color);
    }
}

void RefreshReplayToggleInteractivity(bool interactable) {
    SetSettingInteractable(keepFailToggle, interactable);
    SetSettingInteractable(keepExitToggle, interactable);
    SetSettingInteractable(keepPracticeToggle, interactable);
    SetSettingInteractable(saveEveryAttemptToggle, interactable);
}

void RefreshReplayToggleInteractivity() {
    RefreshReplayToggleInteractivity(getModConfig().SaveLocalReplays.GetValue());
}

void UpdateUI(optional<Player> player) {
    if (spinner) {
        spinner->imageView->get_gameObject()->SetActive(false);
        spinner->SetAnimating(false);
    }

    if (nameField) {
        if (player != nullopt) {
            nameField->SetText(player->name + ", hi!", true);
            nameField->get_gameObject()->SetActive(true);
        } else {
            nameField->get_gameObject()->SetActive(false);
        }
    }

    if (label3) {
        label3->get_gameObject()->SetActive(player == nullopt);
    }

    if (logoutButton) {
        logoutButton->get_gameObject()->SetActive(player != nullopt);
    }

    if (loginField) {
        loginField->get_gameObject()->SetActive(player == nullopt);
        if (player == nullopt) {
            loginField->set_interactable(true);
        }
    }

    if (passwordField) {
        passwordField->get_gameObject()->SetActive(player == nullopt);
        if (player == nullopt) {
            passwordField->set_interactable(true);
        }
    }

    if (loginButton) {
        loginButton->get_gameObject()->SetActive(player == nullopt);
        if (player == nullopt) {
            loginButton->set_interactable(true);
        }
    }

    if (signupButton) {
        signupButton->get_gameObject()->SetActive(player == nullopt);
        if (player == nullopt) {
            signupButton->set_interactable(true);
        }
    }

    if (errorDescriptionLabel) {
        errorDescriptionLabel->SetText(errorDescription, true);
        errorDescriptionLabel->get_gameObject()->SetActive(!errorDescription.empty());
    }

    RefreshReplayToggleInteractivity();
    RefreshCurrentTabLayout();
}

void ShowLoading() {
    if (spinner) {
        spinner->imageView->get_gameObject()->SetActive(true);
        spinner->SetAnimating(true);
    }

    if (loginField) {
        loginField->set_interactable(false);
    }
    if (passwordField) {
        passwordField->set_interactable(false);
    }
    if (loginButton) {
        loginButton->set_interactable(false);
    }
    if (signupButton) {
        signupButton->set_interactable(false);
    }
}

void BuildAccountTab(UnityEngine::Transform* parent) {
    nameField = ::BSML::Lite::CreateText(parent, "");
    EmojiSupport::AddSupport(nameField);

    logoutButton = ::BSML::Lite::CreateUIButton(parent, "Logout", []() {
        PlayerController::LogOut();
        UpdateUI(nullopt);
    });

    loginField = ::BSML::Lite::CreateStringSetting(parent, "Login", "", [](StringW value) {
        login = static_cast<string>(value);
    });

    passwordField = ::BSML::Lite::CreateStringSetting(parent, "Password", "", [](StringW value) {
        password = static_cast<string>(value);
    });

    loginButton = ::BSML::Lite::CreateUIButton(parent, "Log in", []() {
        if (login.empty() || password.empty()) {
            errorDescription = "Enter a username and/or password!";
            UpdateUI(nullopt);
            return;
        }

        ShowLoading();
        PlayerController::LogIn(login, password, [](std::optional<Player> const& player, string error) {
            BSML::MainThreadScheduler::Schedule([player, error] {
                if (player == nullopt) {
                    errorDescription = error;
                } else {
                    errorDescription.clear();
                    login.clear();
                    loginField->SetText("");
                    password.clear();
                    passwordField->SetText("");
                }
                UpdateUI(player);
            });
        });
    });

    signupButton = ::BSML::Lite::CreateUIButton(parent, "Sign up", []() {
        if (login.empty() || password.empty()) {
            errorDescription = "Enter a username and/or password!";
            UpdateUI(nullopt);
            return;
        }

        ShowLoading();
        PlayerController::SignUp(login, password, [](std::optional<Player> const& player, string error) {
            BSML::MainThreadScheduler::Schedule([player, error] {
                if (player == nullopt) {
                    errorDescription = error;
                } else {
                    errorDescription.clear();
                    login.clear();
                    loginField->SetText("");
                    password.clear();
                    passwordField->SetText("");
                }
                UpdateUI(player);
            });
        });
    });

    errorDescriptionLabel = QuestUI::CreateText(parent, "");
    errorDescriptionLabel->set_color(UnityEngine::Color(1.0f, 0.0f, 0.0f, 1.0f));

    label3 = QuestUI::CreateText(
        parent,
        "Don't have BeatLeader Quest profile? Sign up with any new login/password.\nYou can change details or merge with the Steam profile on the website."
    );
    label3->set_fontSize(3.0f);
}

void BuildGeneralTab(UnityEngine::Transform* parent) {
    serverDropdown = AddConfigValueDropdownEnum(parent, getModConfig().DomainType, serverOptions);
    serverDropdown->dropdown->add_didSelectCellWithIdxEvent(
        custom_types::MakeDelegate<System::Action_2<UnityW<HMUI::DropdownWithTableView>, int>*>(
            (function<void(UnityW<HMUI::DropdownWithTableView>, int)>)[](auto, auto) {
                WebUtils::refresh_urls();
            }
        )
    );

    starsDropdown = AddConfigValueDropdownEnum(parent, getModConfig().StarValueToShow, starValueOptions);
    starsDropdown->dropdown->add_didSelectCellWithIdxEvent(
        custom_types::MakeDelegate<System::Action_2<UnityW<HMUI::DropdownWithTableView>, int>*>(
            (function<void(UnityW<HMUI::DropdownWithTableView>, int)>)[](auto, auto) {
                BeatLeader::LeaderboardHeaderManagerNS::Instance.RefreshMapStatus();
            }
        )
    );

    if (ReplayInstalled()) {
        showReplaySettingsToggle = AddConfigValueToggle(parent, getModConfig().ShowReplaySettings);
    }

    noticeboardToggle = AddConfigValueToggle(parent, getModConfig().NoticeboardEnabled);
}

void BuildReplaysTab(UnityEngine::Transform* parent) {
    keepReplaysToggle = AddConfigValueToggle(parent, getModConfig().SaveLocalReplays);
    keepFailToggle = AddConfigValueToggle(parent, getModConfig().KeepFailReplays);
    keepExitToggle = AddConfigValueToggle(parent, getModConfig().KeepExitReplays);
    keepPracticeToggle = AddConfigValueToggle(parent, getModConfig().KeepPracticeReplays);
    saveEveryAttemptToggle = AddConfigValueToggle(parent, getModConfig().SaveEveryReplayAttempt);

    auto* toggle = keepReplaysToggle ? keepReplaysToggle->GetComponentInChildren<UnityEngine::UI::Toggle*>() : nullptr;
    if (toggle && toggle->onValueChanged) {
        toggle->onValueChanged->AddListener(
            custom_types::MakeDelegate<UnityEngine::Events::UnityAction_1<bool>*>(
                (function<void(bool)>)[](bool value) {
                    RefreshReplayToggleInteractivity(value);
                }
            )
        );
    }

    auto* replayNote = QuestUI::CreateText(
        parent,
        "Turning off 'Save every attempt' keeps only the latest replay.",
        false
    );
    replayNote->set_fontSize(3.0f);
    replayNote->set_color(UnityEngine::Color(0.7f, 0.7f, 0.7f, 1.0f));
}

void SetActiveTab(PreferencesTab tab) {
    currentTab = static_cast<int>(tab);

    if (accountTabTransform) {
        accountTabTransform->get_gameObject()->SetActive(tab == PreferencesTab::Account);
    }
    if (generalTabTransform) {
        generalTabTransform->get_gameObject()->SetActive(tab == PreferencesTab::General);
    }
    if (replaysTabTransform) {
        replaysTabTransform->get_gameObject()->SetActive(tab == PreferencesTab::Replays);
    }

    RefreshCurrentTabLayout();
    UpdateUI(PlayerController::currentPlayer);
}

void SetActiveTab(int tabIndex) {
    switch (tabIndex) {
        case 0:
            SetActiveTab(PreferencesTab::Account);
            break;
        case 1:
            SetActiveTab(PreferencesTab::General);
            break;
        case 2:
        default:
            SetActiveTab(PreferencesTab::Replays);
            break;
    }
}

void BeatLeader::PreferencesViewController::DidActivate(bool firstActivation, bool addedToHeirarchy, bool screenSystemDisabling) {
    if (firstActivation) {
        this->get_gameObject()->AddComponent<HMUI::Touchable*>();

        auto* container = BSML::Lite::CreateScrollableSettingsContainer(this->get_transform());
        contentContainerTransform = container->get_transform();

        auto spinnerImage = ::BSML::Lite::CreateImage(this->get_transform(), BundleLoader::bundle->beatLeaderLogoGradient, {0, 20}, {20, 20});
        spinner = this->get_gameObject()->AddComponent<BeatLeader::LogoAnimation*>();
        spinner->Init(spinnerImage);
        spinnerImage->get_gameObject()->SetActive(false);

        accountTabTransform = CreateTabPage(contentContainerTransform, u"BeatLeaderPreferencesAccount");
        generalTabTransform = CreateTabPage(contentContainerTransform, u"BeatLeaderPreferencesGeneral");
        replaysTabTransform = CreateTabPage(contentContainerTransform, u"BeatLeaderPreferencesReplays");

        BuildAccountTab(accountTabTransform);
        BuildGeneralTab(generalTabTransform);
        BuildReplaysTab(replaysTabTransform);

        tabSelectorContainerTransform = CreateAnchoredContainer(
            this->get_transform(),
            u"BeatLeaderPreferencesTabSelector",
            {0.0f, TabSelectorY},
            {TabSelectorWidth, TabSelectorHeight}
        );

        ArrayW<StringW> tabNames(3);
        tabNames[0] = u"Account";
        tabNames[1] = u"General";
        tabNames[2] = u"Replays";
        tabSelector = QuestUI::CreateTextSegmentedControl(
            tabSelectorContainerTransform,
            {0.0f, 0.0f},
            {TabSelectorWidth, TabSelectorHeight},
            tabNames,
            [](int index) {
                SetActiveTab(index);
            }
        );

        if (tabSelectorContainerTransform) {
            tabSelectorContainerTransform->SetAsLastSibling();
        }
    }

    if (tabSelector && tabSelector->segmentedControl) {
        tabSelector->segmentedControl->SelectCellWithNumber(currentTab);
    }

    SetActiveTab(static_cast<PreferencesTab>(currentTab));
}

void BeatLeader::PreferencesViewController::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
    errorDescription.clear();
}
