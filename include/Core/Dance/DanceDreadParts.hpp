#pragma once

#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"

#include <cmath>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace DanceDread {
    extern float TrailDroop;
    extern float NoteGray;
    extern float NoteDim;

    #pragma region Math

    inline float Clamp01(float value) {
        return value < 0 ? 0 : value > 1 ? 1 : value;
    }

    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * Clamp01(t);
    }

    inline float InverseLerp(float a, float b, float value) {
        return a != b ? Clamp01((value - a) / (b - a)) : 0;
    }

    inline float SmoothStep01(float t) {
        t = Clamp01(t);
        return t * t * (3 - 2 * t);
    }

    inline float CenterDistance(float u, float v) {
        float dx = u * 2 - 1, dy = v * 2 - 1;
        return std::sqrt(dx * dx + dy * dy);
    }

    float RandomValue();
    float RandomRange(float min, float max);
    int RandomRange(int min, int maxExclusive);

    inline UnityEngine::Vector3 Add(UnityEngine::Vector3 a, UnityEngine::Vector3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    inline UnityEngine::Vector3 Scale(UnityEngine::Vector3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    inline UnityEngine::Vector3 LerpVector(UnityEngine::Vector3 a, UnityEngine::Vector3 b, float t) {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }

    inline UnityEngine::Vector3 AroundY(float degrees) {
        float radians = degrees * (float)M_PI / 180;
        return {std::sin(radians), 0, std::cos(radians)};
    }

    UnityEngine::Quaternion LookRotation(UnityEngine::Vector3 forward, UnityEngine::Vector3 up = {0, 1, 0});
    UnityEngine::Quaternion Slerp(UnityEngine::Quaternion a, UnityEngine::Quaternion b, float t);

    inline UnityEngine::Color WithAlpha(UnityEngine::Color color, float alpha) {
        return {color.r, color.g, color.b, alpha};
    }

    template <typename T>
    T ReadValue(void const* data) {
        T value;
        std::memcpy(&value, data, sizeof(T));
        return value;
    }

    #pragma endregion

    #pragma region Engine

    template <typename T>
    T* Alive(SafePtrUnity<T> const& pointer) {
        return pointer ? pointer.ptr() : nullptr;
    }

    template <typename T>
    void Destroy(SafePtrUnity<T>& object) {
        if (object) UnityEngine::Object::Destroy(object.ptr());
        object = nullptr;
    }

    float DeltaTime();
    float GameTime();
    UnityEngine::Vector3 GetPosition(UnityEngine::Transform* transform);

    UnityEngine::Transform* Head();
    UnityEngine::Vector3 HeadPosition();
    void ForgetHead();

    void Face(UnityEngine::Transform* transform, UnityEngine::Vector3 head);

    #pragma endregion

    #pragma region UI images

    inline constexpr float ImageScale = 0.01f;

    struct WorldImage {
        UnityEngine::GameObject* root;
        UnityEngine::UI::Image* image;
    };

    WorldImage CreateImage(std::string_view name, UnityEngine::Sprite* sprite, UnityEngine::Vector2 size, UnityEngine::Material* material);
    UnityEngine::UI::Image* AddImage(std::string_view name, UnityEngine::Sprite* sprite, UnityEngine::Transform* parent, UnityEngine::Material* material);
    UnityEngine::Sprite* CreateSprite(UnityEngine::Texture2D* texture, UnityEngine::Vector2 pivot);
    UnityEngine::Sprite* CreateAlphaSprite(int width, int height, UnityEngine::Vector2 pivot, std::vector<float> const& alpha);
    UnityEngine::Sprite* CreateAlphaSprite(int width, int height, UnityEngine::Vector2 pivot, std::function<float(float u, float v)> const& alphaAt);
    UnityEngine::Material* NoGlowMaterial();

    #pragma endregion

    namespace Lights {
        void BeginLevel();
        void CollectStaticLights();
        void Apply(float dim, float tint, float desaturation);
        void Reset();
    }

    namespace Sounds {
        void Preload();
        UnityEngine::AudioClip* Heartbeat();
        UnityEngine::AudioClip* Sting();
        UnityEngine::AudioClip* Relief();
        UnityEngine::AudioClip* RandomWhisper();
        UnityEngine::AudioClip* Voice(std::string_view name);
    }

    namespace Apparitions {
        void Preload();
        void Update(float silhouetteStage, float eyesStage, int stops, float nearDeath, float dt);
        void Dispose();
    }

    namespace Ghost {
        void Load(std::string const& hash, std::string const& difficulty, std::string const& mode);
        void Update(float stage, float songTime);
        void Dispose();
    }

    void ShowDeathText();
}
