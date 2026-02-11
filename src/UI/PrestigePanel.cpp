#include "UI/PrestigePanel.hpp"
#include "API/PlayerController.hpp"
#include "UI/LeaderboardUI.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include <chrono>
#include <thread>
#include "include/Utils/WebUtils.hpp"
#include "main.hpp"

DEFINE_TYPE(BeatLeader, PrestigePanelComponent);

namespace BeatLeader {
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
        switch (state) {
            case ReplayUploadStatus::finished: {
                if (player.level == 100) {
                LocalComponent()->_PrestigeYesButton->interactable = true;
                } else {
                LocalComponent()->_PrestigeYesButton->interactable = false;
                }
                break;
            }
            default:
                return;
        }
    }

    void PrestigePanel::OnUploadStateChanged(std::optional<ScoreUpload> scoreUpload, ReplayUploadStatus state) {
        switch (state) {
            case ReplayUploadStatus::finished:
                if (scoreUpload != std::nullopt && scoreUpload->status != ScoreUploadStatus::Error && scoreUpload->score != std::nullopt) {
                    Player player = scoreUpload->score->player;
                    if (player.level == 100) {
                        LocalComponent()->_PrestigeYesButton->interactable = true;
                    } else {
                        LocalComponent()->_PrestigeYesButton->interactable = false;
                    }
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

    StringW PrestigePanel::GetContent() {
        return StringW(R"(
<vertical spacing="1" pad="1" vertical-fit="PreferredSize" bg="round-rect-panel">
    <macro.define name="image-width" value="18"/>
    <horizontal pref-height="20">
        <vertical spacing="0.5" vertical-fit="PreferredSize">
            <horizontal pref-height="5">
                <text text="Do you want to Prestige? This will reset your level" font-size="5" font-color="#999999" align="Center"/>
            </horizontal>
            <horizontal pref-height="5">
                <text text="You must be max level (100) to Prestige" font-size="5" font-color="#999999" align="Center"/>
            </horizontal>
            <horizontal pref-height="5" spacing="2" horizontal-fit="PreferredSize">
                <button pref-width="16" id="_PrestigeYesButton" text="Yes"/>
                <button pref-width="16" id="_PrestigeNoButton" text="No"/>
            </horizontal>
        </vertical>
    </horizontal>
</vertical>
        )");
    }
} // namespace BeatLeader

namespace BeatLeader::PrestigePanelStatic {
    invokable<> RequestStateChanged;
}