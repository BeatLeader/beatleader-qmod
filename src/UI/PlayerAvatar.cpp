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

DEFINE_TYPE(BeatLeader, PlayerAvatar);

void BeatLeader::PlayerAvatar::Init(HMUI::ImageView* imageView) {
    this->imageView = imageView;

    this->materialInstance = UnityEngine::Object::Instantiate(BundleLoader::playerAvatarMaterial);
    this->AvatarTexturePropertyId = UnityEngine::Shader::PropertyToID(il2cpp_utils::createcsstr("_AvatarTexture"));
    this->FadeValuePropertyId = UnityEngine::Shader::PropertyToID(il2cpp_utils::createcsstr("_FadeValue"));
    imageView->set_material(this->materialInstance);
}

void BeatLeader::PlayerAvatar::SetPlayer(Il2CppString* url, Il2CppString* roles) {
    materialInstance->SetTexture(FadeValuePropertyId, 0);
    play = false;
    currentFrame = 0;
    frameTime = 0;

    auto self = this;
    Sprites::get_AnimatedIcon(to_utf8(csstrtostr(url)), [self, roles](AllFramesResult result) {
        self->imageView->set_sprite(UnityEngine::Sprite::Create(
                result.frames->get(0), 
                UnityEngine::Rect(0.0, 0.0, (float)result.frames->get(0)->get_width(), (float)result.frames->get(0)->get_height()), 
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

        schemeForRole(to_utf8(csstrtostr(roles))).Apply(self->materialInstance);
    });
}

// Stolen from Nya: https://github.com/FrozenAlex/Nya-utils :lovege:
void BeatLeader::PlayerAvatar::Update() {
  if (play) {
    int length = animationFrames->Length();
    if (length > 0) {
      float deltaTime = UnityEngine::Time::get_deltaTime();

      bool isFrameNeeded = false;

      // Count frame length
      float frameLength = animationTimings->get(currentFrame) / 100;
      if (frameLength > 0.0f) {
        // Basic delta time based frame switching
        while (frameTime > frameLength) {
          currentFrame = (currentFrame + 1) % length;
          isFrameNeeded = true;
          frameTime = frameTime - frameLength;
          frameLength = animationTimings->get(currentFrame) / 100;
        }
      } else {
        // Skip the frame with 0 ms
        currentFrame = (currentFrame + 1) % length;
        isFrameNeeded = true;
        frameLength = animationTimings->get(currentFrame) / 100;
      }

      if (isFrameNeeded) {
        if (animationFrames->Length() > currentFrame) {
          auto frame = animationFrames->get(currentFrame);

          if (frame != nullptr) {
            materialInstance->SetTexture(AvatarTexturePropertyId, frame);
          }
        }
      }
      frameTime += deltaTime;
    }
  }
}