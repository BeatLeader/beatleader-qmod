#pragma once

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Object.hpp"
#include "Models/SerializablePose.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include <vector>

namespace BeatLeader::UnityExtensions {

    // template<typename T>
    // T* GetOrAddComponent(UnityEngine::GameObject* go) where T : UnityEngine::Component* {
    //     T* component;
    //     if (!go->TryGetComponent<T>(component)) {
    //         component = go->AddComponent<T>();
    //     }
    //     return component;
    // }

    bool TryDestroy(UnityEngine::Object* obj);

    // template<typename T>
    // std::vector<T*> GetChildren(UnityEngine::Transform* transform, bool firstLevel = true) where T : UnityEngine::Component* {
    //     auto components = transform->GetComponentsInChildren<T>(true);
    //     std::vector<T*> result;
        
    //     for (auto component : components) {
    //         auto t = component->get_transform();
    //         if (t && t != transform && (!firstLevel || t->get_parent() == transform)) {
    //             result.push_back(component);
    //         }
    //     }
    //     return result;
    // }

    void SetLocalPose(UnityEngine::Transform* transform, UnityEngine::Pose pose);
    void SetLocalPose(UnityEngine::Transform* transform, SerializablePose pose);
    UnityEngine::Pose GetLocalPose(UnityEngine::Transform* transform);
    SerializablePose Lerp(SerializablePose a, SerializablePose b, float f);
}; 