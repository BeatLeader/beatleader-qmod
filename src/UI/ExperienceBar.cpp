#include "UI/ExperienceBar.hpp"
#include "API/PlayerController.hpp"
#include "Assets/BundleLoader.hpp"
#include "Models/Player.hpp"
#include "Models/Score.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Mathf.hpp"
#include "Utils/ModConfig.hpp"
#include <optional>
#include <string>
#include "Utils/ReplayManager.hpp"
#include "main.hpp"
#include "UI/Abstract/ReeModalSystem.hpp"
#include "UI/PrestigePanel.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

using namespace UnityEngine;

DEFINE_TYPE(BeatLeader, ExperienceBarComponent);

namespace BeatLeader {
    namespace {
        bool firstInProgressHasRun = false;
    }

    void ExperienceBarComponent::ctor() {
    }

    void ExperienceBarComponent::Update() {
        if (_initialized && _level != 100 && (_isIdle || _isAnimated)) {
            _elapsedTime += Time::get_deltaTime();
            if (_isIdle) {
                // Idle highlight animation, slowly pulses the highlight value
                float t = Mathf::Clamp01(_elapsedTime / _animationDuration);
                if (!_reverse) {
                    _highlight = Mathf::Lerp(0.0f, 1.0f, t);
                } else {
                    _highlight = Mathf::Lerp(1.0f, 0.0f, t);
                }

                if (_elapsedTime >= _animationDuration) {
                    _elapsedTime = 0.0f;
                    _reverse = !_reverse;
                }
            }
            else if (_isAnimated) {
                // Experience filling the bar animation with wave effect
                if (_levelUpValue > 0) { // Level up animation
                    _elapsedTime2 += Time::get_deltaTime();
                    float t = Mathf::Clamp01(_elapsedTime2 * (_levelUpValue + 1) / _animationDuration);
                    float targetValue = 1 - _expProgress;
                    if (_levelUpCount != 0) { // Before final level
                        _sessionProgress = Mathf::Lerp(0.0f, targetValue, t);
                    } else { // Final level
                        _sessionProgress = Mathf::Lerp(0.0f, _targetValue, t);
                    }
                    // Consider the number of level ups to speed up the animation
                    if (_elapsedTime2 * (_levelUpValue + 1) >= _animationDuration) {
                      if (_levelUpCount != 0) { // Reset for next level up
                        _levelUpCount--;
                        SetLevelText(++_level);
                        _expProgress = 0.0f;
                        _sessionProgress = 0.0f;
                        _elapsedTime2 = 0.0f;
                      }
                    }
                } else { // Non-level up animation
                    _gradientT = Mathf::Clamp01(_elapsedTime / _animationDuration);
                    _sessionProgress = Mathf::Lerp(0.0f, _targetValue, _gradientT);
                }

                // Forcefully end animation if time exceeded
                if (_elapsedTime >= _animationDuration) {
                  _level += _levelUpCount; // Add leftover level ups if still existing
                  SetLevelText(_level);
                  _sessionProgress = _targetValue;
                  _isAnimated = false;
                }
            }

            SetMaterialProperties();
        }
    }

    void ExperienceBarComponent::SetMaterialProperties() {
        static const int GradientTPropertyId = UnityEngine::Shader::PropertyToID("_GradientT");
        static const int ExpProgressPropertyId = UnityEngine::Shader::PropertyToID("_ExpProgress");
        static const int SessionProgressPropertyId = UnityEngine::Shader::PropertyToID("_SessionProgress");
        static const int HighlightPropertyId = UnityEngine::Shader::PropertyToID("_Highlight");
        if (_level == 100) {
            _expProgress = 1;
            _sessionProgress = 0.0f;
        }

        _materialInstance->SetFloat(GradientTPropertyId, _gradientT);
        _materialInstance->SetFloat(ExpProgressPropertyId, _expProgress);
        _materialInstance->SetFloat(SessionProgressPropertyId, _sessionProgress);
        _materialInstance->SetFloat(HighlightPropertyId, _highlight);
    }

    void ExperienceBar::OnInitialize() 
    {
        LocalComponent()->_initialized = false;
        SetMaterial();
        LocalComponent()->_hoverHint = BSML::Lite::AddHoverHint(LocalComponent()->_experienceBar, "");
        PlayerController::playerChanged.emplace_back([this](std::optional<Player> optPlayer) {
            this->OnProfileRequestStateChanged(optPlayer.value_or(Player()), optPlayer.has_value() ? ReplayUploadStatus::finished : ReplayUploadStatus::error);
        });
        OnExperienceBarConfigChanged(getModConfig().ExperienceBarEnabled.GetValue());
    }

    void ExperienceBar::OnDispose() 
    {
        // GlobalSettingsView.ExperienceBarConfigEvent -= OnExperienceBarConfigChanged;
        // UserRequest.StateChangedEvent -= OnProfileRequestStateChanged;
        BeatLeader::UploadStateCallback.unhook(uploadStateCallbackFunc);
        BeatLeader::PrestigePanelStatic::RequestStateChanged.unhook(prestigeRequestStateChangedCallbackFunc);
    }

    void ExperienceBarComponent::OnClick() 
    {
        ReeModalSystem::OpenModal<BeatLeader::PrestigePanel>(this->_parent->get_parent()->transform, nullptr);
    }

    void ExperienceBar::OnExperienceBarConfigChanged(bool enabled) 
    {
        LocalComponent()->_experienceBar->gameObject->SetActive(enabled);

        if (enabled && !LocalComponent()->_initialized) {
            BeatLeader::UploadStateCallback.hook(uploadStateCallbackFunc);
            BeatLeader::PrestigePanelStatic::RequestStateChanged.hook(prestigeRequestStateChangedCallbackFunc);
        } else if (!enabled && LocalComponent()->_initialized) {
            BeatLeader::UploadStateCallback.unhook(uploadStateCallbackFunc);
            BeatLeader::PrestigePanelStatic::RequestStateChanged.unhook(prestigeRequestStateChangedCallbackFunc);
        }

        LocalComponent()->_initialized = enabled;
        firstInProgressHasRun = false;

        if (enabled) {
            if (PlayerController::currentPlayer != std::nullopt) {
                ApplyPlayerState(PlayerController::currentPlayer.value());
            } else {
                LocalComponent()->_level = 0;
                LocalComponent()->_prestige = 0;
                LocalComponent()->_currentExperience = 0;
                LocalComponent()->_requiredExp = CalculateRequiredExperience(0, 0);
                LocalComponent()->_expProgress = 0.0f;
                ResetExperienceBarData();
                LocalComponent()->HideLevelText();
            }
        } else {
            ResetExperienceBarData();
            LocalComponent()->_experienceBar->raycastTarget = true;
            LocalComponent()->HideLevelText();
        }
    }

    void ExperienceBar::ApplyPlayerState(Player const& player, bool refreshVisual) 
    {
        LocalComponent()->_level = player.level;
        LocalComponent()->_prestige = player.prestige;
        LocalComponent()->_currentExperience = player.experience;
        LocalComponent()->_experienceBar->raycastTarget = player.prestige != 10;
        LocalComponent()->_requiredExp = CalculateRequiredExperience(player.level, player.prestige);
        LocalComponent()->_expProgress = LocalComponent()->_requiredExp > 0.0f
            ? Mathf::Clamp01(static_cast<float>(player.experience) / LocalComponent()->_requiredExp)
            : 0.0f;

        ResetExperienceBarData(refreshVisual);

        if (LocalComponent()->_initialized) {
            LocalComponent()->SetLevelText(LocalComponent()->_level);
        }
    }

    void ExperienceBar::OnProfileRequestStateChanged(Player player, ReplayUploadStatus state) 
    {
        BSML::MainThreadScheduler::Schedule([this, player, state] {
            if (state == ReplayUploadStatus::finished) {
                ApplyPlayerState(player, getModConfig().ExperienceBarEnabled.GetValue());
            } else if (PlayerController::currentPlayer == std::nullopt) {
                LocalComponent()->_level = 0;
                LocalComponent()->_prestige = 0;
                LocalComponent()->_currentExperience = 0;
                LocalComponent()->_requiredExp = CalculateRequiredExperience(0, 0);
                LocalComponent()->_expProgress = 0.0f;
                LocalComponent()->_experienceBar->raycastTarget = true;
                ResetExperienceBarData(getModConfig().ExperienceBarEnabled.GetValue());
                LocalComponent()->HideLevelText();
            }
        });
    }

    int ExperienceBar::CalculateRequiredExperience(int level, int prestige) 
    {
        int requiredExp = 500 + (50 * level);
        if (prestige != 0) {
            requiredExp = (int)Mathf::Round(requiredExp * Mathf::Pow(1.2f, prestige));
        }
        return requiredExp;
    }

    void ExperienceBarComponent::SetLevelText(int level) {
        _levelTextHolder->set_text(StringW(std::to_string(level)));
        _levelTextHolder->gameObject->SetActive(true);
        if (level != 100) {
            _nextLevelTextHolder->set_text(StringW(std::to_string(level + 1)));
            if (_hoverHint) {
                _hoverHint->set_text(
                    std::to_string(_currentExperience) + " | " +
                    std::to_string(static_cast<int>(_requiredExp)) + " to level " +
                    std::to_string(level + 1)
                );
            }
        } else {
            _nextLevelTextHolder->set_text(StringW("Prestige"));
            if (_hoverHint) {
                _hoverHint->set_text("You can prestige now!");
            }
        }
        _nextLevelTextHolder->gameObject->SetActive(true);
    }

    void ExperienceBarComponent::HideLevelText() {
      _levelTextHolder->gameObject->SetActive(false);
      _nextLevelTextHolder->gameObject->SetActive(false);
      if (_hoverHint) {
          _hoverHint->set_text("");
      }
    }

    void ExperienceBar::ResetExperienceBarData(bool refreshVisual) 
    {
        LocalComponent()->_levelUpCount = 0;
        LocalComponent()->_levelUpValue = 0;
        LocalComponent()->_targetValue = 0;
        LocalComponent()->_sessionProgress = 0;
        LocalComponent()->_gradientT = 0;
        LocalComponent()->_highlight = 0;
        LocalComponent()->_elapsedTime = 0;
        LocalComponent()->_elapsedTime2 = 0;
        LocalComponent()->_isAnimated = false;
        LocalComponent()->_isIdle = false;
        if (refreshVisual) {
            LocalComponent()->SetMaterialProperties();
        }
    }

    void ExperienceBar::OnUploadStateChanged(std::optional<ScoreUpload> scoreUpload, ReplayUploadStatus state) 
    {
        if (LocalComponent()->_level == 100) return;
        bool hadUploadStart = firstInProgressHasRun;

        if (state == ReplayUploadStatus::inProgress && !firstInProgressHasRun) {
            if (LocalComponent()->_levelUpValue == 0) {
                LocalComponent()->_expProgress += LocalComponent()->_targetValue;
            } else {
                LocalComponent()->_expProgress = LocalComponent()->_targetValue;
            }
            ResetExperienceBarData();
            LocalComponent()->_isIdle = true;
            firstInProgressHasRun = true;
        }

        if (state == ReplayUploadStatus::finished && scoreUpload != std::nullopt && scoreUpload->score != std::nullopt) {
            // Clear uploads emit an inProgress event first, but failed attempts do not.
            // When there was no upload-start event, commit the previous animated gain here
            // so the next delta is computed from the correct base progress.
            if (!hadUploadStart) {
                if (LocalComponent()->_levelUpValue == 0) {
                    LocalComponent()->_expProgress += LocalComponent()->_targetValue;
                } else {
                    LocalComponent()->_expProgress = LocalComponent()->_targetValue;
                }
            }

            ResetExperienceBarData();

            Player player = scoreUpload->score->player;
            LocalComponent()->_currentExperience = player.experience;
            if (player.level == LocalComponent()->_level) {
                LocalComponent()->_targetValue = player.experience / LocalComponent()->_requiredExp - LocalComponent()->_expProgress;
                LocalComponent()->SetLevelText(LocalComponent()->_level);
            } else {
                LocalComponent()->_levelUpCount = player.level - LocalComponent()->_level;
                LocalComponent()->_levelUpValue = LocalComponent()->_levelUpCount;
                LocalComponent()->_requiredExp = CalculateRequiredExperience(player.level, player.prestige);
                LocalComponent()->_targetValue = player.experience / LocalComponent()->_requiredExp;
            }

            LocalComponent()->_isAnimated = true;
        }

        if (state != ReplayUploadStatus::inProgress) {
            firstInProgressHasRun = false;
        }
    }

    void ExperienceBar::OnPrestigeRequestStateChanged() 
    {
        if (PlayerController::currentPlayer == std::nullopt) {
            return;
        }

        firstInProgressHasRun = false;
        ApplyPlayerState(PlayerController::currentPlayer.value(), true);
    }

    void ExperienceBar::SetMaterial() 
    {
        LocalComponent()->_materialInstance = Object::Instantiate(BundleLoader::bundle->experienceBarMaterial);
        LocalComponent()->_experienceBar->material = LocalComponent()->_materialInstance;
        LocalComponent()->_experienceBar->set_sprite(BundleLoader::bundle->beatLeaderLogoGradient);
    }

    StringW ExperienceBar::GetContent() {
        return StringW(R"(
            <vertical id="HorizontalLayout">
                <horizontal pref-height="6" pad-top="2" spacing="2">
                    <text id="_levelTextHolder" font-size="3.5" pref-width="3" align="Left" bold="true"/>
                    <clickable-image id="_experienceBar" preserve-aspect="false" pref-width="40" on-click="OnClick"/>
                    <text id="_nextLevelTextHolder" font-size="3.5" pref-width="3" align="Left" bold="true"/>
                </horizontal>
            </vertical>
        )");
    }
} // namespace BeatLeader
