#include "UI/Christmas/ChristmasOrnamentLoader.hpp"
#include "Utils/WebUtils.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "UnityEngine/AssetBundleRequest.hpp"

#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Christmas/ChristmasTreeOrnament.hpp"

#include "logging.hpp"
#include "custom-types/shared/coroutine.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include <thread>
#include <future>
#include <mutex>

namespace BeatLeader {

    map<int, std::vector<uint8_t>> ChristmasOrnamentLoader::bundleCache;
    map<int, UnityEngine::Object*> ChristmasOrnamentLoader::prefabCache;

    void ChristmasOrnamentLoader::LoadOrnamentPrefabAsync(int bundleId, std::function<void(UnityEngine::GameObject*)> callback) {

        if (bundleCache.contains(bundleId)) {
            std::thread([bundleId, callback]() {
                auto bundleData = bundleCache[bundleId];
                ArrayW<uint8_t> bytes(bundleData.size());
                std::copy(bundleData.begin(), bundleData.end(), bytes.begin());

                using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, ArrayW<uint8_t>, int>;
                static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

                auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);

                req->set_allowSceneActivation(true);
                while (!req->get_isDone()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                auto bundle = req->get_assetBundle();
                auto loadRequest = bundle->LoadAllAssetsAsync<UnityEngine::GameObject*>();
                while (!loadRequest->get_isDone()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                UnityEngine::Object* prefab = loadRequest->asset;
                bundle->Unload(false);
                callback(reinterpret_cast<UnityEngine::GameObject*>(prefab));
            }).detach();
            return;
        }

        std::string url = "https://cdn.assets.beatleader.xyz/project_tree_ornament_" + std::to_string(bundleId) + "_android.bundle";
        WebUtils::GetAsync(url, [bundleId, callback](long code, string data) {
            if (code != 200) {
                return;
            }

            std::vector<uint8_t> bundleData(data.begin(), data.end());
            bundleCache[bundleId] = bundleData;
            
            LoadOrnamentPrefabAsync(bundleId, callback);
        });
    }

    custom_types::Helpers::Coroutine ChristmasOrnamentLoader::LoadBundle(int bundleId, ChristmasTree* tree, UnityEngine::Transform* parent, std::function<void(ChristmasTreeOrnament*)> callback)
    {
        auto bundleData = bundleCache[bundleId];

        UnityEngine::Object* prefab = nullptr;
        if (prefabCache.contains(bundleId)) {
            prefab = prefabCache[bundleId];
        } else {
            ArrayW<uint8_t> bytes(bundleData.size());
            std::copy(bundleData.begin(), bundleData.end(), bytes.begin());

            using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, ArrayW<uint8_t>, int>;
            static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

            auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);

            req->set_allowSceneActivation(true);
            co_yield reinterpret_cast<System::Collections::IEnumerator*>(req);

            auto bundle = req->get_assetBundle();
            auto loadRequest = bundle->LoadAllAssetsAsync<UnityEngine::GameObject*>();
            co_yield reinterpret_cast<System::Collections::IEnumerator*>(loadRequest);

            prefab = loadRequest->asset;
            prefabCache[bundleId] = prefab;
            bundle->Unload(false);
        }

        auto go = UnityEngine::Object::Instantiate(reinterpret_cast<UnityEngine::GameObject*>(prefab));
        auto ornament = go->AddComponent<ChristmasTreeOrnament*>();
        ornament->SetupTree(tree, bundleId);
        ornament->Init(parent);

        if (callback) {
            callback(ornament);
        }
        
        co_return;
    }

    void ChristmasOrnamentLoader::Spawn(int id, ChristmasTree* tree, UnityEngine::Transform* parent, std::function<void(ChristmasTreeOrnament*)> callback) {
        if (bundleCache.contains(id)) {
            tree->::UnityEngine::MonoBehaviour::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(LoadBundle(id, tree, parent, callback)));
        } else {
            std::string url = "https://cdn.assets.beatleader.xyz/project_tree_ornament_" + std::to_string(id) + "_android.bundle";
            WebUtils::GetAsync(url, [id, tree, parent, callback](long code, string data) {
                if (code != 200) {
                    return;
                }

                std::vector<uint8_t> bundleData(data.begin(), data.end());
                bundleCache[id] = bundleData;
                BSML::MainThreadScheduler::Schedule([id, tree, parent, callback]() {
                    tree->::UnityEngine::MonoBehaviour::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(LoadBundle(id, tree, parent, callback)));
                });
            });
        }
    }

} // namespace BeatLeader 