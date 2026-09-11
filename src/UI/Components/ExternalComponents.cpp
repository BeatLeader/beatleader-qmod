#include "beatsaber-hook/shared/utils.hpp"
#include "UI/Components/ExternalComponents.hpp"

DEFINE_TYPE(QuestUI, ExternalComponents);

void QuestUI::ExternalComponents::Add(UnityEngine::Component* component) {
    components.push_back(component);
}

UnityEngine::Component* QuestUI::ExternalComponents::GetByType(Il2CppReflectionType* type) {
    if(!type) return nullptr;
    Il2CppClass* clazz = i2c::functions::class_from_system_type(type);
    for(UnityEngine::Component* component : components)
    {
        if (i2c::functions::class_is_assignable_from(clazz, i2c::functions::object_get_class(component)))
            return component;
    }
    return nullptr;
}