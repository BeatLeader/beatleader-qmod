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
#include "UI/RoleColorScheme.hpp"
#include "UI/PlayerAvatar.hpp"
#include "API/PlayerController.hpp"
#include "Assets/Sprites.hpp"
#include "Assets/BundleLoader.hpp"

#include "main.hpp"

using namespace QuestUI;

static int AvatarTexturePropertyId;
static int FadeValuePropertyId;
static int HueShiftPropertyId;
static int SaturationPropertyId;
static int ScalePropertyId;

DEFINE_TYPE(BeatLeader, PlayerAvatar);

void BeatLeader::PlayerAvatar::Init(HMUI::ImageView* imageView) {
    this->imageView = imageView;
    imageView->get_gameObject()->SetActive(false);

    AvatarTexturePropertyId = UnityEngine::Shader::PropertyToID("_AvatarTexture");
    FadeValuePropertyId = UnityEngine::Shader::PropertyToID("_FadeValue");
    HueShiftPropertyId = UnityEngine::Shader::PropertyToID("_HueShift");
    SaturationPropertyId = UnityEngine::Shader::PropertyToID("_Saturation");
    ScalePropertyId = UnityEngine::Shader::PropertyToID("_Scale");
}

void BeatLeader::PlayerAvatar::SetPlayer(
    StringW url, 
    UnityEngine::Material* roleMaterial, 
    float hueShift,
    float saturation) {

    imageView->get_gameObject()->SetActive(true);
    
    if (this->materialInstance == NULL || roleMaterial->get_name() != this->materialName) {
      this->materialInstance = UnityEngine::Object::Instantiate(roleMaterial);
      this->materialName = roleMaterial->get_name();
      imageView->set_material(this->materialInstance);
    }
    
    float scale = materialInstance->GetFloat(ScalePropertyId);
    imageView->get_transform()->set_localScale({scale, scale, scale});
    materialInstance->SetFloat(HueShiftPropertyId, hueShift);
    materialInstance->SetFloat(SaturationPropertyId, saturation);
    play = false;
    currentFrame = 0;
    frameTime = 0;

    if (!Sprites::has_Icon(url)) {
      materialInstance->SetFloat(FadeValuePropertyId, 0);
    }

    auto self = this;
    Sprites::get_AnimatedIcon(url, [self](AllFramesResult result) {
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
        self->materialInstance->SetFloat(FadeValuePropertyId, 1.0f);
        self->play = true;
    });
}

void BeatLeader::PlayerAvatar::SetHiddenPlayer() {
  SetPlayer("https://beatleadercdn.blob.core.windows.net/assets/IncognitoAvatar.png", BundleLoader::bundle->defaultAvatarMaterial, 0, 0);
}

void BeatLeader::PlayerAvatar::HideImage() {
  imageView->get_gameObject()->SetActive(false);
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