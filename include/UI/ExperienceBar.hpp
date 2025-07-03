#pragma once

#include "Models/Player.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Utils/ReplayManager.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/Material.hpp"
#include "main.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, ExperienceBarComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, _experienceBar);
    DECLARE_INSTANCE_FIELD(UnityEngine::Material*, _materialInstance);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, HorizontalLayout);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _levelTextHolder);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _nextLevelTextHolder);
    DECLARE_BSML_PROPERTY(StringW, LevelText);
    DECLARE_BSML_PROPERTY(StringW, NextLevelText);

    DECLARE_CTOR(ctor);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnClick);
    
public:
    const float _animationDuration = 3.0f;

    __declspec(property(get = get_LevelText, put = set_LevelText)) StringW LevelText;
    __declspec(property(get = get_NextLevelText, put = set_NextLevelText)) StringW NextLevelText;

    int _level;
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
};

namespace BeatLeader {

class ExperienceBar : public ReeUIComponentV2<ExperienceBarComponent*> {
public:
    ExperienceBar() : uploadStateCallbackFunc(BeatLeader::UploadStateFunc{std::bind(&ExperienceBar::OnUploadStateChanged, this, std::placeholders::_1, std::placeholders::_2)}) {
    }
    void OnInitialize() override;
    void OnDispose() override;
    StringW GetContent() override;

    // Public methods
    void OnExperienceBarConfigChanged(bool enabled);
    void OnProfileRequestStateChanged(Player player, ReplayUploadStatus state);
    void OnUploadStateChanged(ScoreUpload instance, ReplayUploadStatus state);
    void OnPrestigeRequestStateChanged(rapidjson::Document& instance, ReplayUploadStatus state);

private:
    BeatLeader::UploadStateCallbackFunc uploadStateCallbackFunc;

    int CalculateRequiredExperience(int level, int prestige);
    void ResetExperienceBarData(bool refreshVisual = true);
    void SetMaterial();
};

}  // namespace BeatLeader 