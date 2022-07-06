#include "UI/ClickableImage.hpp"
#include "include/Utils/StringUtils.hpp"

#include "VRUIControls/VRPointer.hpp"

#include "GlobalNamespace/MenuShockwave.hpp"

#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

#include "main.hpp"

DEFINE_TYPE(QuestUI, ClickableImage);

using namespace UnityEngine;

namespace QuestUI
{
    void ClickableImage::ctor()
    {
        INVOKE_CTOR();
        onClickEvent = OnPointerClickEvent();
        pointerEnterEvent = OnPointerEnterEvent();
        pointerExitEvent = OnPointerExitEvent();
        isHighlighted = false;
        highlightColor = UnityEngine::Color(0.60f, 0.80f, 1.0f, 1.0f);
        defaultColor = UnityEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        buttonClickedSignal = nullptr;
        hapticFeedbackController = nullptr;
        hapticFeedbackPresetSO = nullptr;
        
        static auto base_ctor = il2cpp_utils::FindMethod(classof(HMUI::ImageView*), ".ctor");
        if (base_ctor)
        {
            il2cpp_utils::RunMethod(this, base_ctor);
        }
    }

    void ClickableImage::OnPointerClick(EventSystems::PointerEventData* eventData)
    {
        set_isHighlighted(false);
        if (buttonClickedSignal) buttonClickedSignal->Raise();
        onClickEvent.invoke(eventData);
    }

    void ClickableImage::OnPointerEnter(EventSystems::PointerEventData* eventData)
    {
        set_isHighlighted(true);
        pointerEnterEvent.invoke(eventData);
        Vibrate(!VRUIControls::VRPointer::_get__lastControllerUsedWasRight());
    }

    void ClickableImage::OnPointerExit(EventSystems::PointerEventData* eventData)
    {
        set_isHighlighted(false);
        pointerExitEvent.invoke(eventData);
    }

    void ClickableImage::Vibrate(bool left)
    {
        UnityEngine::XR::XRNode node = left ? UnityEngine::XR::XRNode::LeftHand : UnityEngine::XR::XRNode::RightHand;
        if (hapticFeedbackController && hapticFeedbackPresetSO) hapticFeedbackController->PlayHapticFeedback(node, hapticFeedbackPresetSO);
    }

    void ClickableImage::UpdateHighlight()
    {
        set_color(get_isHighlighted() ? get_highlightColor() : get_defaultColor());
    }

    void ClickableImage::set_highlightColor(UnityEngine::Color color)
    {
        highlightColor = color;
        UpdateHighlight();
    }

    UnityEngine::Color ClickableImage::get_highlightColor()
    {
        return highlightColor;
    }

    void ClickableImage::set_defaultColor(UnityEngine::Color color)
    {
        defaultColor = color;
        UpdateHighlight();
    }

    UnityEngine::Color ClickableImage::get_defaultColor()
    {
        return defaultColor;
    }

    bool ClickableImage::get_isHighlighted()
    {
        return isHighlighted;
    }

    void ClickableImage::set_isHighlighted(bool value)
    {
        isHighlighted = value;
        UpdateHighlight();
    }

    ClickableImage::OnPointerClickEvent& ClickableImage::get_onPointerClickEvent()
    {
        return onClickEvent;
    }

    ClickableImage::OnPointerEnterEvent& ClickableImage::get_onPointerEnterEvent()
    {
        return pointerEnterEvent;
    }

    ClickableImage::OnPointerExitEvent& ClickableImage::get_onPointerExitEvent()
    {
        return pointerExitEvent;
    }
}

namespace QuestUI::BeatSaberUI {

    using HapticPresetSO = Libraries::HM::HMLib::VR::HapticPresetSO;
    static SafePtr<HapticPresetSO> hapticFeedbackPresetSO;

    QuestUI::ClickableImage* CreateClickableImage(UnityEngine::Transform* parent, UnityEngine::Sprite* sprite, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick)
    {
        auto go = UnityEngine::GameObject::New_ctor(newcsstr2("QuestUIClickableImage"));

        auto image = go->AddComponent<QuestUI::ClickableImage*>();
        auto mat_UINoGlows = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>();
        UnityEngine::Material* mat_UINoGlow = NULL;
        for (int i = 0; i < mat_UINoGlows->Length(); i++) {
            if (to_utf8(csstrtostr(mat_UINoGlows->get(i)->get_name())) == "UINoGlow") {
                mat_UINoGlow = mat_UINoGlows->get(i);
                break;
            }
        }

        image->set_material(mat_UINoGlow);

        go->get_transform()->SetParent(parent, false);
        image->get_rectTransform()->set_sizeDelta(sizeDelta);
        image->get_rectTransform()->set_anchoredPosition(anchoredPosition);
        image->set_sprite(sprite);

        go->AddComponent<UnityEngine::UI::LayoutElement*>();

        if (onClick)
            image->get_onPointerClickEvent() += [onClick](auto _){ onClick(); };
        
        try
        {
            auto menuShockWave = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::MenuShockwave*>()->get(0);
            auto buttonClickedSignal = menuShockWave->dyn__buttonClickEvents()->get(menuShockWave->dyn__buttonClickEvents()->Length() - 1);
            image->buttonClickedSignal = buttonClickedSignal;
        }
        catch(const std::exception& e)
        {
            getLogger().error("%s", e.what());
        }

        if (!hapticFeedbackPresetSO)
        {
            hapticFeedbackPresetSO.emplace(UnityEngine::ScriptableObject::CreateInstance<HapticPresetSO*>());
            hapticFeedbackPresetSO->duration = 0.01f;
            hapticFeedbackPresetSO->strength = 0.75f;
            hapticFeedbackPresetSO->frequency = 0.5f;
        }

        auto hapticFeedbackController = UnityEngine::Object::FindObjectOfType<GlobalNamespace::HapticFeedbackController*>();
        image->hapticFeedbackController = hapticFeedbackController;
        image->hapticFeedbackPresetSO = (HapticPresetSO*)hapticFeedbackPresetSO;

        return image;
    }

    QuestUI::ClickableImage* CreateClickableImage(UnityEngine::Transform* parent, UnityEngine::Sprite* sprite, std::function<void()> onClick)
    {
        return CreateClickableImage(parent, sprite, {0, 0}, {0, 0}, onClick);
    }
}