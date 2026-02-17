#pragma once

#include "Models/Player.hpp"
#include "PrestigePanel.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Utils/ReplayManager.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/Material.hpp"
#include "main.hpp"
#include <functional>

DECLARE_CLASS_CUSTOM(BeatLeader, ExperienceBarComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, _experienceBar);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, _materialInstance);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, HorizontalLayout);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _levelTextHolder);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _nextLevelTextHolder);

    DECLARE_CTOR(ctor);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnClick);
    
public:
    static constexpr float _animationDuration = 3.0f;

    int _level;
    int _prestige;
    float _gradientT;
    float _expProgress;
    float _sessionProgress;
    float _requiredExp;

    bool _initialized;
    int _levelUpValue;
    int _levelUpCount;
    bool _isIdle;
    bool _reverse;
    bool _isAnimated;
    float _elapsedTime;
    float _elapsedTime2;
    float _targetValue;

    float _highlight;

    void SetMaterialProperties();
    void SetLevelText(int level);
    void HideLevelText();
};

namespace BeatLeader {

class ExperienceBar : public ReeUIComponentV2<ExperienceBarComponent*> {
public:
    ExperienceBar() : 
        uploadStateCallbackFunc(BeatLeader::UploadStateFunc{std::bind(&ExperienceBar::OnUploadStateChanged, this, std::placeholders::_1, std::placeholders::_2)}),
        prestigeRequestStateChangedCallbackFunc(BeatLeader::PrestigePanelStatic::RequestStateChangedFunc(std::bind(&ExperienceBar::OnPrestigeRequestStateChanged, this)))
    {}
    void OnInitialize() override;
    void OnDispose() override;
    StringW GetContent() override;

    // Public methods
    void OnExperienceBarConfigChanged(bool enabled);
    void OnProfileRequestStateChanged(Player player, ReplayUploadStatus state);
    void OnUploadStateChanged(std::optional<ScoreUpload> scoreUpload, ReplayUploadStatus state);
    void OnPrestigeRequestStateChanged();

private:
    BeatLeader::UploadStateCallbackFunc uploadStateCallbackFunc;
    BeatLeader::PrestigePanelStatic::RequestStateChangedCallbackFunc prestigeRequestStateChangedCallbackFunc;

    int CalculateRequiredExperience(int level, int prestige);
    void ResetExperienceBarData(bool refreshVisual = true);
    void SetMaterial();
};

}  // namespace BeatLeader 