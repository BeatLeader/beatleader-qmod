#pragma once

#include "TMPro/TMP_SubMeshUI.hpp"

#include "UnityEngine/Material.hpp"

#include "HMUI/ImageView.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/EventSystems/IEventSystemHandler.hpp"
#include "UnityEngine/EventSystems/IPointerClickHandler.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "UnityEngine/EventSystems/PointerEventData.hpp"
#include "custom-types/shared/macros.hpp"

#include "GlobalNamespace/HapticFeedbackController.hpp"
#include "GlobalNamespace/Signal.hpp"
#include "Libraries/HM/HMLib/VR/HapticPresetSO.hpp"

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#define TextInterfaces                                                                                                                                                                                                                           \
    {                                                                                                                                                                                                                                            \
        classof(UnityEngine::EventSystems::IPointerClickHandler*), classof(UnityEngine::EventSystems::IPointerEnterHandler*), classof(UnityEngine::EventSystems::IPointerExitHandler*), classof(UnityEngine::EventSystems::IEventSystemHandler*) \
    }

___DECLARE_TYPE_WRAPPER_INHERITANCE(QuestUI, ClickableImage, Il2CppTypeEnum::IL2CPP_TYPE_CLASS, HMUI::ImageView, "QuestUI", TextInterfaces, 0, nullptr,
                                    DECLARE_INSTANCE_METHOD(void, set_highlightColor, UnityEngine::Color color);
                                    DECLARE_INSTANCE_METHOD(UnityEngine::Color, get_highlightColor);
                                    DECLARE_INSTANCE_METHOD(void, set_defaultColor, UnityEngine::Color color);
                                    DECLARE_INSTANCE_METHOD(UnityEngine::Color, get_defaultColor);
                                    DECLARE_INSTANCE_METHOD(void, Vibrate, bool left);

                                    DECLARE_OVERRIDE_METHOD(void, OnPointerClick, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerClickHandler::OnPointerClick>::get(), UnityEngine::EventSystems::PointerEventData* eventData);
                                    DECLARE_OVERRIDE_METHOD(void, OnPointerEnter, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter>::get(), UnityEngine::EventSystems::PointerEventData* eventData);
                                    DECLARE_OVERRIDE_METHOD(void, OnPointerExit, il2cpp_utils::il2cpp_type_check::MetadataGetter<&UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit>::get(), UnityEngine::EventSystems::PointerEventData* eventData);

                                    DECLARE_INSTANCE_FIELD(GlobalNamespace::Signal*, buttonClickedSignal);
                                    DECLARE_INSTANCE_FIELD(GlobalNamespace::HapticFeedbackController*, hapticFeedbackController);
                                    DECLARE_INSTANCE_FIELD(Libraries::HM::HMLib::VR::HapticPresetSO*, hapticFeedbackPresetSO);

                                    DECLARE_CTOR(ctor);
                                    public: 
                                    using OnPointerClickEvent = UnorderedEventCallback<UnityEngine::EventSystems::PointerEventData*>;
                                    using OnPointerEnterEvent = UnorderedEventCallback<UnityEngine::EventSystems::PointerEventData*>;
                                    using OnPointerExitEvent = UnorderedEventCallback<UnityEngine::EventSystems::PointerEventData*>;

                                    OnPointerClickEvent & get_onPointerClickEvent();
                                    OnPointerEnterEvent & get_onPointerEnterEvent();
                                    OnPointerExitEvent & get_onPointerExitEvent();
                                    private: 
                                    UnityEngine::Color highlightColor;
                                    UnityEngine::Color defaultColor;
                                    bool isHighlighted;
                                    OnPointerClickEvent onClickEvent;
                                    OnPointerEnterEvent pointerEnterEvent;
                                    OnPointerExitEvent pointerExitEvent;
                                    void UpdateHighlight();
                                    bool get_isHighlighted();
                                    void set_isHighlighted(bool value);)