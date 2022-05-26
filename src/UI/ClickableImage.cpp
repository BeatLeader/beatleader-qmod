#include "UI/ClickableImage.hpp"
#include "VRUIControls/VRPointer.hpp"

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