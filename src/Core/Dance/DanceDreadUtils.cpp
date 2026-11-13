#include "include/Core/Dance/DanceDreadParts.hpp"

#include "HMUI/ImageView.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Vector4.hpp"

#include "bsml/shared/Helpers/getters.hpp"

#include <chrono>
#include <random>

using namespace UnityEngine;

namespace DanceDread {
    #pragma region Math

    static std::mt19937& Random() {
        static std::mt19937 random((uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());
        return random;
    }

    float RandomValue() {
        return std::uniform_real_distribution<float>(0, 1)(Random());
    }

    float RandomRange(float min, float max) {
        return min + (max - min) * RandomValue();
    }

    int RandomRange(int min, int maxExclusive) {
        if (maxExclusive <= min) return min;
        return std::uniform_int_distribution<int>(min, maxExclusive - 1)(Random());
    }

    static Vector3 Cross(Vector3 a, Vector3 b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    static Vector3 Normalized(Vector3 v) {
        float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return length > 1e-6f ? Scale(v, 1 / length) : Vector3(0, 0, 0);
    }

    Quaternion LookRotation(Vector3 forward, Vector3 up) {
        auto f = Normalized(forward);
        auto r = Normalized(Cross(up, f));
        if (r.x == 0 && r.y == 0 && r.z == 0) r = {1, 0, 0};
        auto u = Cross(f, r);
        float m00 = r.x, m01 = u.x, m02 = f.x;
        float m10 = r.y, m11 = u.y, m12 = f.y;
        float m20 = r.z, m21 = u.z, m22 = f.z;
        float trace = m00 + m11 + m22;
        if (trace > 0) {
            float s = std::sqrt(trace + 1) * 2;
            return {(m21 - m12) / s, (m02 - m20) / s, (m10 - m01) / s, 0.25f * s};
        }
        if (m00 > m11 && m00 > m22) {
            float s = std::sqrt(1 + m00 - m11 - m22) * 2;
            return {0.25f * s, (m01 + m10) / s, (m02 + m20) / s, (m21 - m12) / s};
        }
        if (m11 > m22) {
            float s = std::sqrt(1 + m11 - m00 - m22) * 2;
            return {(m01 + m10) / s, 0.25f * s, (m12 + m21) / s, (m02 - m20) / s};
        }
        float s = std::sqrt(1 + m22 - m00 - m11) * 2;
        return {(m02 + m20) / s, (m12 + m21) / s, 0.25f * s, (m10 - m01) / s};
    }

    Quaternion Slerp(Quaternion a, Quaternion b, float t) {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        if (dot < 0) {
            b = {-b.x, -b.y, -b.z, -b.w};
            dot = -dot;
        }
        float wa, wb;
        if (dot > 0.9995f) {
            wa = 1 - t;
            wb = t;
        } else {
            float theta = std::acos(dot);
            float sine = std::sin(theta);
            wa = std::sin((1 - t) * theta) / sine;
            wb = std::sin(t * theta) / sine;
        }
        Quaternion result = {a.x * wa + b.x * wb, a.y * wa + b.y * wb, a.z * wa + b.z * wb, a.w * wa + b.w * wb};
        float length = std::sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
        if (length < 1e-6f) return a;
        return {result.x / length, result.y / length, result.z / length, result.w / length};
    }

    #pragma endregion

    #pragma region Engine

    template <auto Method, typename R, typename... Args>
    static R CallWithoutBoxing(Args... args) {
        static auto method = il2cpp_utils::il2cpp_type_check::MetadataGetter<Method>::methodInfo();
        return reinterpret_cast<R (*)(Args..., MethodInfo const*)>(method->methodPointer)(args..., method);
    }

    float DeltaTime() {
        return CallWithoutBoxing<&UnityEngine::Time::get_deltaTime, float>();
    }

    float GameTime() {
        return CallWithoutBoxing<&UnityEngine::Time::get_time, float>();
    }

    Vector3 GetPosition(Transform* transform) {
        return CallWithoutBoxing<&Transform::get_position, Vector3>(transform);
    }

    static constexpr float HeadSearchInterval = 1;

    static SafePtrUnity<Transform> head;
    static float nextHeadSearch = 0;

    Transform* Head() {
        if (head) return head.ptr();
        float now = GameTime();
        if (now < nextHeadSearch) return nullptr;
        nextHeadSearch = now + HeadSearchInterval;
        auto camera = Camera::get_main();
        if (!camera) return nullptr;
        head = camera->transform;
        return head.ptr();
    }

    Vector3 HeadPosition() {
        auto transform = Head();
        return transform ? GetPosition(transform) : Vector3(0, 1.7f, 0);
    }

    void ForgetHead() {
        head = nullptr;
        nextHeadSearch = 0;
    }

    void Face(Transform* transform, Vector3 headPosition) {
        auto position = GetPosition(transform);
        Vector3 away = {position.x - headPosition.x, 0, position.z - headPosition.z};
        if (away.x * away.x + away.z * away.z > 0.01f) transform->rotation = LookRotation(away);
    }

    #pragma endregion

    #pragma region UI images

    UI::Image* AddImage(std::string_view name, Sprite* sprite, Transform* parent, Material* material) {
        auto gameObject = GameObject::New_ctor(std::string(name));
        // The game's UI materials draw nothing on a plain Image
        UI::Image* image = material ? static_cast<UI::Image*>(gameObject->AddComponent<HMUI::ImageView*>()) : gameObject->AddComponent<UI::Image*>();
        image->transform->SetParent(parent, false);
        image->sprite = sprite;
        image->raycastTarget = false;
        if (material) image->material = material;
        return image;
    }

    WorldImage CreateImage(std::string_view name, Sprite* sprite, Vector2 size, Material* material) {
        auto root = GameObject::New_ctor(std::string(name));
        auto canvas = root->AddComponent<Canvas*>();
        canvas->renderMode = RenderMode::WorldSpace;
        // ImageView writes its curve data to TexCoord2, without it the game's UI shaders collapse the quad to a line
        canvas->additionalShaderChannels = AdditionalCanvasShaderChannels(canvas->additionalShaderChannels.value__ | AdditionalCanvasShaderChannels::TexCoord2.value__);
        root->GetComponent<RectTransform*>()->sizeDelta = {size.x / ImageScale, size.y / ImageScale};
        root->transform->localScale = {ImageScale, ImageScale, ImageScale};
        auto image = AddImage("Image", sprite, root->transform, material);
        auto imageRect = image->rectTransform;
        imageRect->anchorMin = {0, 0};
        imageRect->anchorMax = {1, 1};
        imageRect->sizeDelta = {0, 0};
        return {root, image};
    }

    Sprite* CreateSprite(Texture2D* texture, Vector2 pivot) {
        texture->wrapMode = TextureWrapMode::Clamp;
        texture->hideFlags = HideFlags::DontUnloadUnusedAsset;
        auto sprite = Sprite::Create(texture, Rect(0, 0, texture->width, texture->height), pivot, 100, 0, SpriteMeshType::FullRect, Vector4(0, 0, 0, 0), false);
        sprite->hideFlags = HideFlags::DontUnloadUnusedAsset;
        return sprite;
    }

    Sprite* CreateAlphaSprite(int width, int height, Vector2 pivot, std::vector<float> const& alpha) {
        ArrayW<uint8_t> pixels(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            pixels[i * 4] = 255;
            pixels[i * 4 + 1] = 255;
            pixels[i * 4 + 2] = 255;
            pixels[i * 4 + 3] = (uint8_t)(Clamp01(alpha[i]) * 255);
        }
        auto texture = Texture2D::New_ctor(width, height, TextureFormat::RGBA32, false, false);
        texture->LoadRawTextureData(pixels);
        texture->Apply();
        return CreateSprite(texture, pivot);
    }

    Sprite* CreateAlphaSprite(int width, int height, Vector2 pivot, std::function<float(float u, float v)> const& alphaAt) {
        std::vector<float> alpha(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) alpha[y * width + x] = alphaAt((x + 0.5f) / width, (y + 0.5f) / height);
        }
        return CreateAlphaSprite(width, height, pivot, alpha);
    }

    Material* NoGlowMaterial() {
        static SafePtrUnity<Material> material;
        if (!material) material = BSML::Helpers::GetUINoGlowMat();
        return Alive(material);
    }

    #pragma endregion
}
