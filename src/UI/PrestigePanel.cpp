#include "UI/PrestigePanel.hpp"
#include "API/PlayerController.hpp"
#include "UI/LeaderboardUI.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include <chrono>
#include <thread>
#include "include/Utils/WebUtils.hpp"
#include "main.hpp"

DEFINE_TYPE(BeatLeader, PrestigePanelComponent);

namespace BeatLeader {
    namespace {
        void SetButtonText(UnityEngine::UI::Button* button, std::string const& text) {
            if (!button) {
                return;
            }

            auto* textMesh = button->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
            if (textMesh) {
                textMesh->SetText(text, true);
            }
        }

        void RefreshPanelLayout(PrestigePanelComponent* component) {
            if (!component || !component->_content) {
                return;
            }

            auto* rectTransform = component->_content->GetComponent<UnityEngine::RectTransform*>();
            if (rectTransform) {
                UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(rectTransform);
            }
        }
    }

    void PrestigePanel::OnInitialize() {
        AbstractReeModal<PrestigePanelComponent*>::OnInitialize();
        this->InitializePrestigeButtons();
        PlayerController::playerChanged.emplace_back([this](std::optional<Player> optPlayer) {
            this->OnProfileRequestStateChanged(optPlayer.value_or(Player()), optPlayer.has_value() ? ReplayUploadStatus::finished : ReplayUploadStatus::error);
        });
        // On PC the event instantly returns the last request value. We cant port that so we need to manually call it
        this->OnProfileRequestStateChanged(PlayerController::currentPlayer.value_or(Player()), PlayerController::currentPlayer.has_value() ? ReplayUploadStatus::finished : ReplayUploadStatus::error);
        BeatLeader::UploadStateCallback.hook(uploadStateCallbackFunc);

        fireworksController = UnityEngine::Object::FindObjectOfType<GlobalNamespace::FireworksController*>();
    }

    void PrestigePanel::OnDispose() {
        BeatLeader::UploadStateCallback.unhook(uploadStateCallbackFunc);
    }

    void PrestigePanel::OnProfileRequestStateChanged(Player player,ReplayUploadStatus state) {
        BSML::MainThreadScheduler::Schedule([this, player, state] {
            switch (state) {
                case ReplayUploadStatus::finished: {
                    UpdatePanelContent(player, player.level >= 100);
                    break;
                }
                default:
                    UpdatePanelContent(Player(), false);
                    return;
            }
        });
    }

    void PrestigePanel::OnUploadStateChanged(std::optional<ScoreUpload> scoreUpload, ReplayUploadStatus state) {
        switch (state) {
            case ReplayUploadStatus::finished:
                if (scoreUpload != std::nullopt && scoreUpload->status != ScoreUploadStatus::Error && scoreUpload->score != std::nullopt) {
                    Player player = scoreUpload->score->player;
                    UpdatePanelContent(player, player.level >= 100);
                }
                break;
            default: 
                return;
        }
    }

    void PrestigePanel::RequestPrestige() {
      WebUtils::GetJSONAsync(
          WebUtils::API_URL + "experience/prestige",
          [](long status, bool error, rapidjson::Document const &result) {
            // Refresh the player prestige
            PlayerController::Refresh(0, [](auto player, auto str) {
              BSML::MainThreadScheduler::Schedule(
                  [] { 
                    PrestigePanelStatic::RequestStateChanged.invoke();
                    LeaderboardUI::updatePlayerRank();
                });
            });
          });
      LocalComponent()->_PrestigeYesButton->interactable = false;
      if (fireworksController != NULL) {
        std::thread([this] {
            this->Fireworks(5);
        }).detach();
      }
      this->Close();
    }

    void PrestigePanel::Fireworks(double duration) {
        auto controller = fireworksController;
        BSML::MainThreadScheduler::Schedule([controller] {
            controller->enabled = true;
        });
        std::this_thread::sleep_for(std::chrono::duration<double>(duration));
        BSML::MainThreadScheduler::Schedule([controller] {
            controller->enabled = false;
        });
    }

    void PrestigePanel::InitializePrestigeButtons() {
        LocalComponent()->_PrestigeYesButton->onClick->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this]() {
            this->RequestPrestige();
        }));
        LocalComponent()->_PrestigeYesButton->interactable = false;
        LocalComponent()->_PrestigeNoButton->onClick->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this]() { 
            this->Close(); 
        }));
    }

    void PrestigePanel::UpdatePanelContent(Player const& player, bool canPrestige) {
        auto* component = LocalComponent();
        if (!component) {
            return;
        }

        if (component->_PrimaryText) {
            component->_PrimaryText->SetText(
                canPrestige
                    ? "<b><color=#ffffff>Congratulations!</color></b>\nYou've reached <b>level 100</b> and can now <b>Prestige</b>."
                    : "Gain experience points by playing any maps, even for failing! Reach <b>level 100</b> to be able to <b>prestige</b> into the next iteration.",
                true
            );
        }

        if (component->_SecondaryText) {
            component->_SecondaryText->SetText(
                canPrestige
                    ? "This will reset your level and you will reach the <b>Prestige " + std::to_string(player.prestige + 1) + "</b>. <color=#ffffff>Are you ready?</color>"
                    : "To get more points, pass maps always with 95+% accuracy. But even playing with 90% accuracy will give you almost the full xp for the time played.\n<color=#ffffff>Just play more!</color>",
                true
            );
        }

        if (component->_PrestigeYesButton) {
            component->_PrestigeYesButton->gameObject->SetActive(canPrestige);
            component->_PrestigeYesButton->interactable = canPrestige;
            SetButtonText(component->_PrestigeYesButton, "Prestige");
        }

        if (component->_PrestigeNoButton) {
            SetButtonText(component->_PrestigeNoButton, "Close");
        }

        RefreshPanelLayout(component);
    }

    StringW PrestigePanel::GetContent() {
        return StringW(R"(
<vertical spacing="2" pad="2" pref-width="72" vertical-fit="PreferredSize" bg="round-rect-panel">
    <vertical spacing="1" pref-width="70" vertical-fit="PreferredSize">
        <text id="_PrimaryText" word-wrapping="true" font-size="4.25" font-color="#999999" align="Center"/>
        <text id="_SecondaryText" word-wrapping="true" font-size="3.75" font-color="#999999" align="Center"/>
        <horizontal pref-height="8" spacing="2" horizontal-fit="PreferredSize">
            <button pref-width="20" id="_PrestigeYesButton" text="Prestige"/>
            <button pref-width="20" id="_PrestigeNoButton" text="Close"/>
        </horizontal>
    </vertical>
</vertical>
        )");
    }
} // namespace BeatLeader

namespace BeatLeader::PrestigePanelStatic {
    invokable<> RequestStateChanged;
}
