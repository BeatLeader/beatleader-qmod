#pragma once

#include "UI/ReeUIComponentV2.hpp"

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Vector4.hpp"

#include <string>
#include <string_view>

DECLARE_CLASS_CUSTOM(BeatLeader, SkillTriangleComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _triangleImage);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _textComponentA);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _textComponentB);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _textComponentC);
};

namespace BeatLeader {

class SkillTriangle : public ReeUIComponentV2<SkillTriangleComponent*> {
public:
    void OnInitialize() override;
    void OnDispose() override;
    StringW GetContent() override;

    void SetValues(float techRating, float accRating, float passRating);

private:
    static std::string FormatLabel(std::string_view label, float value);
    void InitializeLabels();
    void InitializeMaterial();
    void UpdateMaterialProperties();

    UnityEngine::Material* _materialInstance = nullptr;
    UnityEngine::Vector4 _normalizedValues = UnityEngine::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
};

}
