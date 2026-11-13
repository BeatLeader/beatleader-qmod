#include "include/Core/Dance/DanceDreadParts.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Utils/WebUtils.hpp"

#include "UnityEngine/RectTransform.hpp"

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "main.hpp"

#include <atomic>
#include <mutex>

using namespace UnityEngine;

namespace DanceDread::Ghost {
    static constexpr Vector3 Position = {-2.2f, 0, 3};
    static constexpr Color GhostColor = {0.55f, 0.85f, 1, 1};
    static constexpr float SaberLength = 1;
    static constexpr float SaberWidth = 0.035f;
    static constexpr float HeadSize = 0.28f;

    struct Pose {
        Vector3 position;
        Quaternion rotation;
    };

    struct GhostFrame {
        float time;
        Pose head, left, right;
    };

    using Frames = std::vector<GhostFrame>;

    #pragma region Replay

    static constexpr int32_t BsorMagic = 0x442d3d69;
    static constexpr int32_t MaxStringLength = 300;
    static constexpr size_t FrameSize = 4 + 4 + 3 * 7 * 4;

    static bool IsPlatformLength(int32_t length) {
        return length == 6 || length == 5 || length == 8;
    }

    class Reader {
    public:
        explicit Reader(std::string const& buffer) : buffer(buffer) {}

        std::string const& buffer;
        size_t position = 0;

        bool Has(size_t count, size_t at) const {
            return at + count <= buffer.size();
        }

        template <typename T>
        T Peek(size_t at) const {
            if (!Has(sizeof(T), at)) throw std::runtime_error("bsor: truncated");
            return ReadValue<T>(buffer.data() + at);
        }

        template <typename T>
        T Next() {
            auto value = Peek<T>(position);
            position += sizeof(T);
            return value;
        }

        void SkipString() {
            auto length = Peek<int32_t>(position);
            while (length < 0 || length > MaxStringLength) length = Peek<int32_t>(++position);
            if (!Has((size_t)length, position + 4)) throw std::runtime_error("bsor: truncated");
            position += 4 + (size_t)length;
        }

        // Old recorders miscounted the bytes of some names, the name runs until the platform's length
        void SkipName() {
            auto length = Peek<int32_t>(position);
            int32_t offset = 0;
            if (length > 0) {
                while (!IsPlatformLength(Peek<int32_t>(position + 4 + (size_t)length + (size_t)offset))) offset++;
            }
            position += 4 + (size_t)(length + offset);
        }

        void SkipInfo() {
            for (int i = 0; i < 4; i++) SkipString();
            SkipName();
            for (int i = 0; i < 8; i++) SkipString();
            Next<int32_t>();
            for (int i = 0; i < 3; i++) SkipString();
            position += 4 + 1 + 4 + 4 + 4 + 4;
        }

        Pose ReadPose() {
            Pose pose;
            pose.position = {Next<float>(), Next<float>(), Next<float>()};
            pose.rotation = {Next<float>(), Next<float>(), Next<float>(), Next<float>()};
            return pose;
        }
    };

    static Frames DecodeFrames(std::string const& bytes) {
        Reader reader(bytes);
        if (reader.Next<int32_t>() != BsorMagic || reader.Next<uint8_t>() != 1) throw std::runtime_error("bsor: bad magic or version");
        if (reader.Next<uint8_t>() != 0) throw std::runtime_error("bsor: no info");
        reader.SkipInfo();
        if (reader.Next<uint8_t>() != 1) throw std::runtime_error("bsor: no frames");

        auto count = reader.Next<int32_t>();
        if (count < 0 || !reader.Has((size_t)count * FrameSize, reader.position)) throw std::runtime_error("bsor: truncated frames");
        Frames frames;
        frames.reserve(count);
        for (int i = 0; i < count; i++) {
            GhostFrame frame;
            frame.time = reader.Next<float>();
            reader.Next<int32_t>();
            frame.head = reader.ReadPose();
            frame.left = reader.ReadPose();
            frame.right = reader.ReadPose();
            if (frame.time != 0 && (frames.empty() || frame.time != frames.back().time)) frames.push_back(frame);
        }
        return frames;
    }

    static std::mutex loadedMutex;
    static Frames loaded;
    static std::atomic<int> generation = 0;

    static void Publish(int requested, Frames frames) {
        std::lock_guard lock(loadedMutex);
        if (requested == generation) loaded = std::move(frames);
    }

    void Load(std::string const& hash, std::string const& difficulty, std::string const& mode) {
        int requested = ++generation;
        {
            std::lock_guard lock(loadedMutex);
            loaded = Frames();
        }
        auto player = PlayerController::currentPlayer ? PlayerController::currentPlayer->id : "";
        auto url = WebUtils::API_URL + "v3/scores/" + hash + "/" + difficulty + "/" + mode + "/dnc/global/page?player=" + player + "&page=1&count=1";
        WebUtils::GetAsync(url, [requested](long status, std::string const& body) {
            if (requested != generation) return;
            if (status != 200) {
                BeatLeaderLogger.info("Dance or Die: no ghost, the top dance score request returned {}", status);
                return;
            }
            rapidjson::Document document;
            document.Parse(body.c_str());
            if (document.HasParseError() || !document.IsObject() || !document.HasMember("data") || !document["data"].IsArray() || document["data"].Empty()) {
                BeatLeaderLogger.info("Dance or Die: no ghost, nobody danced this map yet");
                return;
            }
            auto const& score = document["data"][0];
            if (!score.IsObject() || !score.HasMember("replay") || !score["replay"].IsString()) return;
            std::string replay = score["replay"].GetString();
            if (replay.empty()) return;
            WebUtils::GetAsync(replay, [requested](long status, std::string const& bytes) {
                if (requested != generation) return;
                if (status != 200) {
                    BeatLeaderLogger.info("Dance or Die: no ghost, the replay download returned {}", status);
                    return;
                }
                try {
                    auto frames = DecodeFrames(bytes);
                    if (frames.size() < 2) return;
                    BeatLeaderLogger.info("Dance or Die: the ghost has {} frames", frames.size());
                    Publish(requested, std::move(frames));
                } catch (std::exception const& e) {
                    BeatLeaderLogger.warn("Dance or Die: can't read the ghost replay: {}", e.what());
                }
            });
        });
    }

    #pragma endregion

    #pragma region Visuals

    static Sprite* DiscSprite() {
        static SafePtrUnity<Sprite> sprite;
        if (!sprite) {
            sprite = CreateAlphaSprite(64, 64, {0.5f, 0.5f}, [](float u, float v) {
                float distance = CenterDistance(u, v);
                return Clamp01((1 - distance) * 4) * 0.7f + Clamp01(1 - distance) * 0.3f;
            });
        }
        return Alive(sprite);
    }

    static Sprite* BarSprite() {
        static SafePtrUnity<Sprite> sprite;
        if (!sprite) {
            sprite = CreateAlphaSprite(8, 32, {0.5f, 0.5f}, [](float, float v) {
                return Clamp01((1 - std::abs(v * 2 - 1)) * 2.5f);
            });
        }
        return Alive(sprite);
    }

    static Frames frames;
    static size_t frameIndex = 0;
    static SafePtrUnity<GameObject> root;
    static SafePtrUnity<Transform> head;
    static SafePtrUnity<Transform> headBillboard;
    static SafePtrUnity<Transform> leftSaber;
    static SafePtrUnity<Transform> rightSaber;
    static std::vector<SafePtrUnity<UI::Image>> images;
    static float appliedAlpha = -1;
    static bool visible = false;

    static Transform* CreatePart(std::string_view name, Transform* parent) {
        auto part = GameObject::New_ctor(std::string(name))->transform;
        part->SetParent(parent, false);
        return part;
    }

    static Transform* CreateSaber(std::string_view name, Transform* parent) {
        auto saber = CreatePart(name, parent);
        Quaternion crossedBlades[] = {
            LookRotation({-1, 0, 0}, {0, 1, 0}),
            LookRotation({0, 1, 0}, {1, 0, 0})
        };
        for (auto rotation : crossedBlades) {
            auto [blade, image] = CreateImage("Blade", BarSprite(), {SaberLength, SaberWidth}, nullptr);
            blade->GetComponent<RectTransform*>()->pivot = {0, 0.5f};
            blade->transform->SetParent(saber, false);
            blade->transform->localRotation = rotation;
            images.emplace_back(image);
        }
        return saber;
    }

    static void Spawn() {
        images.clear();
        auto ghost = GameObject::New_ctor("DanceDreadGhost");
        ghost->transform->position = Position;
        root = ghost;
        auto headPart = CreatePart("Head", ghost->transform);
        auto [billboard, headImage] = CreateImage("HeadGlow", DiscSprite(), {HeadSize, HeadSize}, nullptr);
        billboard->transform->SetParent(headPart, false);
        images.emplace_back(headImage);
        head = headPart;
        headBillboard = billboard->transform;
        leftSaber = CreateSaber("LeftSaber", ghost->transform);
        rightSaber = CreateSaber("RightSaber", ghost->transform);
        appliedAlpha = -1;
        visible = true;
    }

    static void ApplyPose(Transform* target, Pose const& a, Pose const& b, float t) {
        target->localPosition = LerpVector(a.position, b.position, t);
        target->localRotation = Slerp(a.rotation, b.rotation, t);
    }

    static void ApplyFrame(float songTime) {
        if (frames[frameIndex].time > songTime) frameIndex = 0;
        while (frameIndex < frames.size() - 2 && frames[frameIndex + 1].time <= songTime) frameIndex++;
        auto const& a = frames[frameIndex];
        auto const& b = frames[frameIndex + 1];
        float t = b.time > a.time ? Clamp01((songTime - a.time) / (b.time - a.time)) : 0;
        ApplyPose(head.ptr(), a.head, b.head, t);
        ApplyPose(leftSaber.ptr(), a.left, b.left, t);
        ApplyPose(rightSaber.ptr(), a.right, b.right, t);
        auto billboard = headBillboard.ptr();
        auto position = GetPosition(billboard);
        auto viewer = HeadPosition();
        billboard->rotation = LookRotation({position.x - viewer.x, position.y - viewer.y, position.z - viewer.z});
    }

    #pragma endregion

    void Update(float stage, float songTime) {
        if (frames.empty()) {
            std::lock_guard lock(loadedMutex);
            if (loaded.empty()) return;
            frames.swap(loaded);
            frameIndex = 0;
        }
        if (!root) {
            if (stage <= 0) return;
            Spawn();
        }
        bool show = stage > 0.02f;
        if (show != visible) {
            visible = show;
            root->SetActive(show);
        }
        if (!show) return;
        float alpha = 0.8f * stage;
        if (std::abs(alpha - appliedAlpha) > 0.004f) {
            appliedAlpha = alpha;
            for (auto& image : images) image->color = WithAlpha(GhostColor, alpha);
        }
        ApplyFrame(songTime);
    }

    void Dispose() {
        generation++;
        {
            std::lock_guard lock(loadedMutex);
            loaded = Frames();
        }
        frames = Frames();
        frameIndex = 0;
        Destroy(root);
        head = nullptr;
        headBillboard = nullptr;
        leftSaber = nullptr;
        rightSaber = nullptr;
        images.clear();
        visible = false;
    }
}
