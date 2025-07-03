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

using namespace UnityEngine;

DEFINE_TYPE(BeatLeader, ExperienceBarComponent);

namespace BeatLeader {

    void ExperienceBarComponent::ctor() {
        _LevelText = StringW("");
        _NextLevelText = StringW("");
        BeatLeaderLogger.warn("CTOR BL LOG");
    }

    void ExperienceBarComponent::Update() {
        BeatLeaderLogger.warn("UPDATE BL LOG");
        if (_initialized && _level != 100 && (_isIdle || _isAnimated)) {
            if (_isIdle) {
                _elapsedTime += Time::get_deltaTime();
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
                _elapsedTime2 += Time::get_deltaTime();
                _gradientT = Mathf::Clamp01(_elapsedTime2 / _animationDuration);
                if (_levelUpValue > 0) {
                    _elapsedTime += Time::get_deltaTime();
                    float t = Mathf::Clamp01(_elapsedTime * (_levelUpValue + 1) / _animationDuration);
                    float targetValue = 1 - _expProgress;
                    if (_levelUpCount != 0) {
                        _sessionProgress = Mathf::Lerp(0.0f, targetValue, t);
                    } else {
                        _sessionProgress = Mathf::Lerp(0.0f, _targetValue, t);
                    }

                    if (_elapsedTime * (_levelUpValue + 1) >= _animationDuration) {
                        if (_levelUpCount == 0) {
                            _sessionProgress = _targetValue;
                            _elapsedTime = 0.0f;
                            _isAnimated = false;
                        } else {
                            _levelUpCount--;
                            SetLevelText(_level++);
                            _expProgress = 0.0f;
                            _sessionProgress = 0.0f;
                            _elapsedTime = 0.0f;
                        }
                    }
                } else {
                    _sessionProgress = Mathf::Lerp(0, _targetValue, _gradientT);

                    if (_elapsedTime >= _animationDuration)
                    {
                        _sessionProgress = _targetValue;
                        _isAnimated = false;
                    }
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
        // GlobalSettingsView.ExperienceBarConfigEvent += OnExperienceBarConfigChanged;
        BeatLeaderLogger.warn("REGISTERING BL");
        PlayerController::playerChanged.emplace_back([this](std::optional<Player> optPlayer) {
            this->OnProfileRequestStateChanged(optPlayer.value_or(Player()), optPlayer.has_value() ? ReplayUploadStatus::finished : ReplayUploadStatus::error);
        });
        if (getModConfig().ExperienceBarEnabled.GetValue()) {
            BeatLeader::UploadStateCallback.hook(uploadStateCallbackFunc);
            // PrestigeRequest.StateChangedEvent += OnPrestigeRequestStateChanged;
        } else {
            LocalComponent()->LevelText = StringW("");
            LocalComponent()->NextLevelText = StringW("");
            LocalComponent()->_experienceBar->gameObject->SetActive(false);
        }
    }

    void ExperienceBar::OnDispose() 
    {
        // GlobalSettingsView.ExperienceBarConfigEvent -= OnExperienceBarConfigChanged;
        // UserRequest.StateChangedEvent -= OnProfileRequestStateChanged;
        BeatLeader::UploadStateCallback.unhook(uploadStateCallbackFunc);
        // PrestigeRequest.StateChangedEvent -= OnPrestigeRequestStateChanged;
    }

    void ExperienceBarComponent::OnClick() 
    {
        // TODO Prestige
    }

    void ExperienceBar::OnExperienceBarConfigChanged(bool enabled) 
    {
        LocalComponent()->_experienceBar->gameObject->SetActive(enabled);
        ResetExperienceBarData();
        if (enabled && !LocalComponent()->_initialized) {
            BeatLeader::UploadStateCallback.hook(uploadStateCallbackFunc);
            // PrestigeRequest.StateChangedEvent += OnPrestigeRequestStateChanged;
            LocalComponent()->SetLevelText(LocalComponent()->_level);
            LocalComponent()-> _initialized = enabled;
        } else if (LocalComponent()->_initialized) {
            BeatLeader::UploadStateCallback.unhook(uploadStateCallbackFunc);
            // UploadReplayRequest.StateChangedEvent -= OnUploadStateChanged;
            // PrestigeRequest.StateChangedEvent -= OnPrestigeRequestStateChanged;
            LocalComponent()->LevelText = StringW("");
            LocalComponent()->NextLevelText = StringW("");
            LocalComponent()->_initialized = enabled;
        }
    }

    void ExperienceBar::OnProfileRequestStateChanged(Player player, ReplayUploadStatus state) 
    {
        if (!LocalComponent()->_initialized && state == ReplayUploadStatus::finished) {
            LocalComponent()->_level = player.level;
            LocalComponent()->SetLevelText(LocalComponent()->_level);
            LocalComponent()->_requiredExp = CalculateRequiredExperience(player.level, player.prestige);
            LocalComponent()->_expProgress = player.experience / LocalComponent()->_requiredExp;
            ResetExperienceBarData();
            if (getModConfig().ExperienceBarEnabled.GetValue()) {
                LocalComponent()->_experienceBar->gameObject->SetActive(getModConfig().ExperienceBarEnabled.GetValue());
                LocalComponent()->_initialized = true;
            }
        }
    }

    int ExperienceBar::CalculateRequiredExperience(int level, int prestige) 
    {
        int requiredExp = 500 + (50 * level);
        if (prestige != 0) {
            requiredExp = (int)Mathf::Round(requiredExp * Mathf::Pow(1.33f, prestige));
        }
        return requiredExp;
    }

    void ExperienceBarComponent::SetLevelText(int level) {
        LevelText = StringW(std::to_string(level));
        if (level != 100) {
            NextLevelText = StringW(std::to_string(_level + 1));
        } else {
            NextLevelText = StringW("Prestige");
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

    void ExperienceBar::OnUploadStateChanged(ScoreUpload scoreUpload, ReplayUploadStatus state) 
    {
        if (LocalComponent()->_level == 100) return;

        // TODO uh not really the same. Maybe another value?
        if (state == ReplayUploadStatus::inProgress) {
            if (LocalComponent()->_levelUpValue == 0) {
                LocalComponent()->_expProgress += LocalComponent()->_targetValue;
            } else {
                LocalComponent()->_expProgress = LocalComponent()->_targetValue;
            }
            ResetExperienceBarData();
            LocalComponent()->_isIdle = true;
        }

        if (state == ReplayUploadStatus::finished) {
            LocalComponent()->_isIdle = false;
            LocalComponent()->_highlight = 0;

            if (scoreUpload.Status != ScoreUploadStatus::Error) {
                Player player = scoreUpload.Score.player;
                if (player.level == LocalComponent()->_level) {
                    LocalComponent()->_targetValue = player.experience / LocalComponent()->_requiredExp - LocalComponent()->_expProgress;
                    LocalComponent()->_levelUpValue = 0;
                } else {
                    LocalComponent()->_levelUpCount = player.level - LocalComponent()->_level;
                    LocalComponent()->_levelUpValue = LocalComponent()->_levelUpCount;
                    LocalComponent()->_requiredExp = CalculateRequiredExperience(player.level, player.prestige);
                    LocalComponent()->_targetValue = player.experience / LocalComponent()->_requiredExp;
                }

                LocalComponent()->_isAnimated = true;
            }
        }
    }

    void ExperienceBar::OnPrestigeRequestStateChanged(rapidjson::Document& instance, ReplayUploadStatus state) 
    {
        if (state == ReplayUploadStatus::finished) {
            LocalComponent()->_level = 0;
            LocalComponent()->SetLevelText(LocalComponent()->_level);
            LocalComponent()->_expProgress = 0;
            ResetExperienceBarData();
        }
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
                    <text id="_levelTextHolder" text="~LevelText" font-size="3.5" pref-width="3" align="Left" bold="true"/>
                    <clickable-image id="_experienceBar" preserve-aspect="false" pref-width="40" on-click="OnClick"/>
                    <text id="_nextLevelTextHolder" text="~NextLevelText" font-size="3.5" pref-width="3" align="Left" bold="true"/>
                </horizontal>
            </vertical>
        )");
    }

    StringW ExperienceBarComponent::get_LevelText() { return _LevelText; }

    void ExperienceBarComponent::set_LevelText(StringW value) {
      _LevelText = value;
      _levelTextHolder->set_text(value);
    }

    StringW ExperienceBarComponent::get_NextLevelText() { return _NextLevelText; }

    void ExperienceBarComponent::set_NextLevelText(StringW value) {
      _NextLevelText = value;
      _nextLevelTextHolder->set_text(value);
    }
} // namespace BeatLeader