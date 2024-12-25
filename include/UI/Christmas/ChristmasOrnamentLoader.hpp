#pragma once

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/AssetBundle.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "custom-types/shared/coroutine.hpp"

#include <map>

namespace BeatLeader {

class ChristmasTree;
class ChristmasTreeOrnament;

class ChristmasOrnamentLoader {
public:
    static void LoadOrnamentPrefabAsync(int bundleId, std::function<void(UnityEngine::GameObject*)> callback);
    static custom_types::Helpers::Coroutine LoadBundle(int bundleId, ChristmasTree* tree, UnityEngine::Transform* parent, std::function<void(ChristmasTreeOrnament*)> callback);
    static void Spawn(int id, ChristmasTree* tree, UnityEngine::Transform* parent, std::function<void(ChristmasTreeOrnament*)> callback);

private:
    static std::map<int, std::vector<uint8_t>> bundleCache;
    static std::map<int, UnityEngine::Object*> prefabCache;
};

} // namespace BeatLeader 