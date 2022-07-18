#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Time.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "API/PlayerController.hpp"
#include "Assets/BundleLoader.hpp"

#include "include/UI/LogoAnimation.hpp"

#include "main.hpp"

#include <string>
#include <cmath>

using namespace QuestUI;
using namespace std;

DEFINE_TYPE(BeatLeader, LogoAnimation);

static int GlowPropertyId;
static int DotScalePropertyId;
static int BlockScalePropertyId;
static int CornerRadiusPropertyId;
static int ThicknessPropertyId;
static int SpinnerRotationPropertyId;
static int SpinnerAmplitudePropertyId;

const float IdleGlow = 0.15f;
const float IdleDotScale = 0.25f;
const float IdleBlockScale = 0.7f;
const float IdleCornerRadius = 0.2f;
const float IdleThickness = 0.08f;
const float IdleFill = 1.0f;

const float ThinkingGlow = 0.9f;
const float ThinkingDotScale = 0.1f;
const float ThinkingBlockScale = 0.5f;
const float ThinkingCornerRadius = 0.5f;
const float ThinkingThickness = 0.16f;
const float ThinkingFill = 0.4f;
const float ThinkingRotationSpeed = 12.0f;

float _glow = IdleGlow;
float _dotScale = IdleDotScale;
float _blockScale = IdleBlockScale;
float _cornerRadius = IdleCornerRadius;
float _thickness = IdleThickness;
float _fill = IdleFill;
float _targetSpinnerRotation;
float _spinnerRotation;

bool _isThinking;
const float HalfPI = 1.570796f;

void BeatLeader::LogoAnimation::Init(HMUI::ImageView* imageView) {
    this->imageView = imageView;

    this->materialInstance = UnityEngine::Object::Instantiate(BundleLoader::bundle->logoMaterial);
    imageView->set_material(this->materialInstance);

    GlowPropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_Glow"));
    DotScalePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_DotScale"));
    BlockScalePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_BlockScale"));
    CornerRadiusPropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_CornerRadius"));
    ThicknessPropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_Thickness"));
    SpinnerRotationPropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_SpinnerRotation"));
    SpinnerAmplitudePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_SpinnerAmplitude"));
}

void BeatLeader::LogoAnimation::SetAnimating(bool animating) {
    if (_isThinking == animating) return;
    _isThinking = animating;

    if (animating) return;
    _targetSpinnerRotation = ceil(_spinnerRotation / HalfPI) * HalfPI;
}

void BeatLeader::LogoAnimation::Update() {
    float deltaTime = UnityEngine::Time::get_deltaTime();
    float slowT = deltaTime * 10.0f;
    float fastT = deltaTime * 20.0f;

    if (_isThinking) {
        _glow = lerp(_glow, ThinkingGlow, fastT);
        _dotScale = lerp(_dotScale, ThinkingDotScale, slowT);
        _blockScale = lerp(_blockScale, ThinkingBlockScale, slowT);
        _cornerRadius = lerp(_cornerRadius, ThinkingCornerRadius, slowT);
        _thickness = lerp(_thickness, ThinkingThickness, slowT);
        _fill = lerp(_fill, ThinkingFill, slowT);
        _spinnerRotation += ThinkingRotationSpeed * deltaTime;
    } else {
        _glow = lerp(_glow, IdleGlow, fastT);
        _dotScale = lerp(_dotScale, IdleDotScale, slowT);
        _blockScale = lerp(_blockScale, IdleBlockScale, slowT);
        _cornerRadius = lerp(_cornerRadius, IdleCornerRadius, slowT);
        _thickness = lerp(_thickness, IdleThickness, slowT);
        _fill = lerp(_fill, IdleFill, fastT);
        _spinnerRotation = lerp(_spinnerRotation, _targetSpinnerRotation, fastT);
    }
    
    materialInstance->SetFloat(GlowPropertyId, _glow);
    materialInstance->SetFloat(DotScalePropertyId, _dotScale);
    materialInstance->SetFloat(BlockScalePropertyId, _blockScale);
    materialInstance->SetFloat(CornerRadiusPropertyId, _cornerRadius);
    materialInstance->SetFloat(ThicknessPropertyId, _thickness);
    materialInstance->SetFloat(SpinnerAmplitudePropertyId, _fill);
    materialInstance->SetFloat(SpinnerRotationPropertyId, _spinnerRotation);
}