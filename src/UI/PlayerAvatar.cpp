#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/SpriteMeshType.hpp"

#include "Utils/ModConfig.hpp"
#include "Utils/WebUtils.hpp"
#include "API/PlayerController.hpp"

#include "include/UI/PlayerAvatar.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"

#include "main.hpp"

using namespace QuestUI;

DEFINE_TYPE(BeatLeader, PlayerAvatar);

struct ColorScheme {
    int BackgroundColorPropertyId;
    int RimColorPropertyId;
    int HaloColorPropertyId;
    int WavesAmplitudePropertyId;

    UnityEngine::Color _backgroundColor;
    UnityEngine::Color _rimColor;
    UnityEngine::Color _haloColor;
    float _wavesAmplitude;

    ColorScheme(UnityEngine::Color backgroundColor, UnityEngine::Color rimColor, UnityEngine::Color haloColor, float wavesAmplitude) {
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

ColorScheme schemeForRole(string roles) {
    if (roles.find("tipper") != string::npos) {
        return ColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.7f, 1.0f),
            UnityEngine::Color(1.0f, 0.6f, 0.0f, 1.0f),
            0.3f
        );
    } else if (roles.find("supporter") != string::npos) {
        return ColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.7f, 1.0f),
            UnityEngine::Color(1.0f, 0.6f, 0.0f, 1.0f),
            0.8f
        );
    } else if (roles.find("sponsor") != string::npos) {
        return ColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.1f, 0.4f),
            UnityEngine::Color(1.0f, 1.0f, 0.6f, 1.0f),
            UnityEngine::Color(1.0f, 0.3f, 0.0f, 1.0f),
            1.0f
        );
    } else {
        return ColorScheme(
            UnityEngine::Color(0.0f, 0.0f, 0.1f, 0.3f),
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 1.0f),
            UnityEngine::Color(0.0f, 0.0f, 0.0f, 1.0f),
            0.0f
        );
    }
}

void BeatLeader::PlayerAvatar::Init(HMUI::ImageView* imageView) {
    this->imageView = imageView;

    this->materialInstance = UnityEngine::Object::Instantiate(BundleLoader::playerAvatarMaterial);
    this->AvatarTexturePropertyId = UnityEngine::Shader::PropertyToID("_AvatarTexture");
    this->FadeValuePropertyId = UnityEngine::Shader::PropertyToID("_FadeValue");
    imageView->set_material(this->materialInstance);
}

void BeatLeader::PlayerAvatar::SetPlayer(StringW url, StringW roles) {
    materialInstance->SetTexture(FadeValuePropertyId, 0);
    play = false;
    currentFrame = 0;
    frameTime = 0;

    auto self = this;
    Sprites::get_AnimatedIcon(url, [self, roles](AllFramesResult result) {
        self->imageView->set_sprite(UnityEngine::Sprite::Create(
                result.frames[0], 
                UnityEngine::Rect(0.0, 0.0, (float)result.frames[0]->get_width(), (float)result.frames[0]->get_height()), 
                UnityEngine::Vector2(0.5, 0.5), 
                100.0, 
                0, 
                UnityEngine::SpriteMeshType::FullRect, 
                UnityEngine::Vector4(0.0, 0.0, 0.0, 0.0), 
                false));
        self->animationFrames = result.frames;
        self->animationTimings = result.timings;
        self->materialInstance->SetFloat(self->FadeValuePropertyId, 1.0f);
        self->play = true;

        schemeForRole(roles).Apply(self->materialInstance);
    });
}

// Stolen from Nya: https://github.com/FrozenAlex/Nya-utils :lovege:
void BeatLeader::PlayerAvatar::Update() {
  if (play) {
    int length = animationFrames.Length();
    if (length > 0) {
      float deltaTime = UnityEngine::Time::get_deltaTime();

      bool isFrameNeeded = false;

      // Count frame length
      float frameLength = animationTimings[currentFrame] / 100;
      if (frameLength > 0.0f) {
        // Basic delta time based frame switching
        while (frameTime > frameLength) {
          currentFrame = (currentFrame + 1) % length;
          isFrameNeeded = true;
          frameTime = frameTime - frameLength;
          frameLength = animationTimings[currentFrame] / 100;
        }
      } else {
        // Skip the frame with 0 ms
        currentFrame = (currentFrame + 1) % length;
        isFrameNeeded = true;
        frameLength = animationTimings[currentFrame] / 100;
      }

      if (isFrameNeeded) {
        if (animationFrames.Length() > currentFrame) {
          auto frame = animationFrames.get(currentFrame);

          if (frame != nullptr) {
            materialInstance->SetTexture(AvatarTexturePropertyId, frame);
          }
        }
      }
      frameTime += deltaTime;
    }
  }
}