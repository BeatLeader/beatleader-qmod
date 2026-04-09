#include "include/UI/SkillTriangle.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/Utils/FormatUtils.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Vector3.hpp"

#include <algorithm>
#include <string>
#include <string_view>

DEFINE_TYPE(BeatLeader, SkillTriangleComponent);

namespace {
    constexpr float MaxRating = 15.0f;
    auto NormalizedValuesPropertyId = UnityEngine::Shader::PropertyToID("_Normalized");
}

namespace BeatLeader {

    void SkillTriangle::OnInitialize() {
        InitializeMaterial();
        InitializeLabels();
    }

    void SkillTriangle::OnDispose() {
        if (_materialInstance) {
            UnityEngine::Object::Destroy(_materialInstance);
            _materialInstance = nullptr;
        }
    }

    StringW SkillTriangle::GetContent() {
        return StringW(R"(
            <image id="_triangleImage" preserve-aspect="true" pref-height="30" pref-width="30">
                <text ignore-layout="true" id="_textComponentA" text="" font-size="3.4" align="Center"/>
                <text ignore-layout="true" id="_textComponentB" text="" font-size="3.4" align="Center"/>
                <text ignore-layout="true" id="_textComponentC" text="" font-size="3.4" align="Center"/>
            </image>
        )");
    }

    void SkillTriangle::SetValues(float techRating, float accRating, float passRating) {
        LocalComponent()->_textComponentA->SetText(FormatLabel("Tech", techRating), true);
        LocalComponent()->_textComponentB->SetText(FormatLabel("Acc", accRating), true);
        LocalComponent()->_textComponentC->SetText(FormatLabel("Pass", passRating), true);

        _normalizedValues.x = std::clamp(techRating / MaxRating, 0.0f, 1.0f);
        _normalizedValues.y = std::clamp(accRating / MaxRating, 0.0f, 1.0f);
        _normalizedValues.z = std::clamp(passRating / MaxRating, 0.0f, 1.0f);
        _normalizedValues.w = 0.0f;
        UpdateMaterialProperties();
    }

    std::string SkillTriangle::FormatLabel(std::string_view label, float value) {
        return std::string(label) + ": " + FormatUtils::FormatStars(value);
    }

    void SkillTriangle::InitializeLabels() {
        LocalComponent()->_textComponentA->get_transform()->set_localPosition(UnityEngine::Vector3(-12.0f, 12.0f, 0.0f));
        LocalComponent()->_textComponentB->get_transform()->set_localPosition(UnityEngine::Vector3(12.0f, 12.0f, 0.0f));
        LocalComponent()->_textComponentC->get_transform()->set_localPosition(UnityEngine::Vector3(0.0f, -12.0f, 0.0f));
    }

    void SkillTriangle::InitializeMaterial() {
        LocalComponent()->_triangleImage->set_sprite(BundleLoader::bundle->beatLeaderLogoGradient);
        _materialInstance = UnityEngine::Material::Instantiate(BundleLoader::bundle->skillTriangleMaterial);
        LocalComponent()->_triangleImage->set_material(_materialInstance);
        UpdateMaterialProperties();
    }

    void SkillTriangle::UpdateMaterialProperties() {
        if (_materialInstance) {
            _materialInstance->SetVector(NormalizedValuesPropertyId, _normalizedValues);
        }
    }

}
