#pragma once

#include "UnityEngine/Material.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Shader.hpp"

#include <string>
using namespace std;

struct RoleColorScheme {
    int BackgroundColorPropertyId;
    int RimColorPropertyId;
    int HaloColorPropertyId;
    int WavesAmplitudePropertyId;

    UnityEngine::Color _backgroundColor;
    UnityEngine::Color _rimColor;
    UnityEngine::Color _haloColor;
    float _wavesAmplitude;

    RoleColorScheme(
        UnityEngine::Color backgroundColor, 
        UnityEngine::Color rimColor, 
        UnityEngine::Color haloColor, 
        float wavesAmplitude) {
        BackgroundColorPropertyId = UnityEngine::Shader::PropertyToID("_BackgroundColor");
        RimColorPropertyId = UnityEngine::Shader::PropertyToID("_RimColor");
        HaloColorPropertyId = UnityEngine::Shader::PropertyToID("_HaloColor");
        WavesAmplitudePropertyId = UnityEngine::Shader::PropertyToID("_WavesAmplitude");
        _backgroundColor = backgroundColor;
        _rimColor = rimColor;
        _haloColor = haloColor;
        _wavesAmplitude = wavesAmplitude;
    }

    void Apply(UnityEngine::Material* material) {
        material->SetColor(BackgroundColorPropertyId, _backgroundColor);
        material->SetColor(RimColorPropertyId, _rimColor);
        material->SetColor(HaloColorPropertyId, _haloColor);
        material->SetFloat(WavesAmplitudePropertyId, _wavesAmplitude);
    }
};

RoleColorScheme inline schemeForRole(string roles, bool avatar = true) {
    if (roles.find("tipper") != string::npos) {
        return RoleColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.7f, 1.0f),
            UnityEngine::Color(1.0f, 0.6f, 0.0f, 1.0f),
            0.3f
        );
    } else if (roles.find("supporter") != string::npos) {
        return RoleColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.7f, 1.0f),
            UnityEngine::Color(1.0f, 0.6f, 0.0f, 1.0f),
            0.8f
        );
    } else if (roles.find("sponsor") != string::npos) {
        return RoleColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.1f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.6f, 1.0f),
            UnityEngine::Color(1.0f, 0.3f, 0.0f, 1.0f),
            1.0f
        );
    } else {
        if (avatar) {
            return RoleColorScheme(
                UnityEngine::Color(0.0f, 0.0f, 0.1f, 0.3f),
                UnityEngine::Color(0.0f, 0.0f, 0.0f, 1.0f),
                UnityEngine::Color(0.0f, 0.0f, 0.0f, 1.0f),
                0.0f
            );
        } else {
            return RoleColorScheme(
                UnityEngine::Color(0.0f, 0.0f, 0.1f, 0.3f),
                UnityEngine::Color(0.4f, 0.6f, 1.0f, 1.0f),
                UnityEngine::Color(0.5f, 0.7f, 1.0f, 1.0f),
                0.0f
            );
        }
        
    }
}

float inline idleHighlight(string roles) {
    if (roles.find("sponsor") != string::npos) {
        return 0.8f;
    } else {
        return 0;
    }
}