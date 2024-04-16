#pragma once
#include <vector>
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Component.hpp"

#include "custom-types/shared/macros.hpp"

DECLARE_CLASS_CODEGEN(QuestUI, ExternalComponents, UnityEngine::MonoBehaviour,

    private:
        std::vector<UnityEngine::Component*> components;
    public:
        template<class T = UnityEngine::Component*>
        T Get() {
            return (T)GetByType(csTypeOf(T));
        }

    DECLARE_INSTANCE_METHOD(void, Add, UnityEngine::Component* component); 
    DECLARE_INSTANCE_METHOD(UnityEngine::Component*, GetByType, Il2CppReflectionType* type);
    
    DECLARE_DEFAULT_CTOR();

    DECLARE_SIMPLE_DTOR();

)