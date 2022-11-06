#include "System/Reflection/BindingFlags.hpp"
#include "System/Reflection/PropertyInfo.hpp"
#include "System/Reflection/FieldInfo.hpp"
#include "System/Type.hpp"
#include "System/Object.hpp"

#include "UI/UIUtils.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/ViewController_AnimationDirection.hpp"

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "main.hpp"

#include "UnityEngine/HideFlags.hpp"

namespace UIUtils {

    SafePtrUnity<HMUI::ImageView> roundRectSprite;

    HMUI::ImageView* getRoundRectSprite() {
        if (!roundRectSprite) {
            roundRectSprite = QuestUI::ArrayUtil::First(UnityEngine::Resources::FindObjectsOfTypeAll<HMUI::ImageView*>(), [](HMUI::ImageView* image){ 
                auto sprite = image->get_sprite();
                if (!sprite || sprite->get_name() != "RoundRect10") return false;

                auto parent = image->get_transform()->get_parent();
                if (!parent || parent->get_name() != "Wrapper") return false;

                auto goName = image->get_gameObject()->get_name();
                if (goName != "KeyboardWrapper") return false;

                return true;
            });
        }

        return roundRectSprite.ptr();
    }

    HMUI::ImageView* GetCopyOf(HMUI::ImageView* comp, HMUI::ImageView* other) {
        // auto type = comp->GetType();

        // System::Reflection::BindingFlags flags =
        //     System::Reflection::BindingFlags::Public | 
        //     System::Reflection::BindingFlags::NonPublic | 
        //     System::Reflection::BindingFlags::Instance;

        // auto pinfos = type->GetProperties(flags);
        // for (int i = 0; i < pinfos.Length(); i++)
        // {
        //     auto pinfo = pinfos[i];
        //     if (pinfo->get_CanWrite() && pinfo->get_Name() != "name")
        //     {
        //         IL2CPP_CATCH_HANDLER(
        //             ArrayW<::Il2CppObject *> frames = ArrayW<::Il2CppObject *>((il2cpp_array_size_t)0);
        //             auto value = pinfo->GetValue(other);
        //             pinfo->SetValue(comp, value, nullptr);
        //         )
        //     }
        // }

        // auto finfos = type->GetFields(flags);
        // for (int i = 0; i < finfos.Length(); i++) {
        //     auto finfo = finfos[i];
        //     finfo->SetValue(comp, finfo->GetValue(other));
        // }

        return comp;
    }

    void OpenSettings() {
        auto currentFlowCoordinator = QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
        
        auto modSettingsFlowCoordinator = QuestUI::GetModSettingsFlowCoordinator();
        if (modSettingsFlowCoordinator == NULL) {
            modSettingsFlowCoordinator = QuestUI::BeatSaberUI::CreateFlowCoordinator(reinterpret_cast<System::Type*>(il2cpp_utils::GetSystemType(il2cpp_utils::GetClassFromName("QuestUI", "ModSettingsFlowCoordinator"))));
        }
        
        currentFlowCoordinator->PresentFlowCoordinator(modSettingsFlowCoordinator, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, true, false);

        QuestUI::MainThreadScheduler::Schedule([modSettingsFlowCoordinator] {
            auto buttons = modSettingsFlowCoordinator->get_topViewController()->GetComponentsInChildren<UnityEngine::UI::Button*>();
            for (size_t i = 0; i < buttons.size(); i++)
            {
                auto textMesh = buttons[i]->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
                if (textMesh->get_text() == "bl" || textMesh->get_text() == "BeatLeader") {
                    buttons[i]->get_onClick()->Invoke();
                }
            }
        });
    }

    // Copied from BSML
    HMUI::ImageView* CreateRoundRectImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
        static ConstString name("QuestUIImage");
        UnityEngine::GameObject* gameObj = UnityEngine::GameObject::New_ctor(name);
        HMUI::ImageView* background = gameObj->AddComponent<HMUI::ImageView*>();// GetCopyOf(, getRoundRectSprite());
        auto bgTemplate = getRoundRectSprite();
        background->set_alphaHitTestMinimumThreshold(bgTemplate->get_alphaHitTestMinimumThreshold());
		background->set_color(bgTemplate->get_color());
		background->set_color0(bgTemplate->get_color0());
		background->set_color1(bgTemplate->get_color1());
		background->set_gradient(bgTemplate->get_gradient());
		background->gradientDirection = bgTemplate->gradientDirection;
        background->flipGradientColors = bgTemplate->flipGradientColors;
        background->skew = bgTemplate->skew;
		background->set_eventAlphaThreshold(bgTemplate->get_eventAlphaThreshold());
		background->set_fillAmount(bgTemplate->get_fillAmount());
		background->set_fillCenter(bgTemplate->get_fillCenter());
		background->set_fillClockwise(bgTemplate->get_fillClockwise());
		background->set_fillMethod(bgTemplate->get_fillMethod());
		background->set_fillOrigin(bgTemplate->get_fillOrigin());
		background->set_hideFlags(bgTemplate->get_hideFlags());
		background->set_maskable(bgTemplate->get_maskable());
		background->set_material(bgTemplate->get_material());
		background->set_onCullStateChanged(bgTemplate->get_onCullStateChanged());
		background->set_overrideSprite(bgTemplate->get_overrideSprite());
		background->set_pixelsPerUnitMultiplier(bgTemplate->get_pixelsPerUnitMultiplier());
		background->set_preserveAspect(bgTemplate->get_preserveAspect());
		background->set_raycastTarget(bgTemplate->get_raycastTarget());
		background->set_sprite(bgTemplate->get_sprite());
		background->set_tag(bgTemplate->get_tag());
		background->set_type(bgTemplate->get_type());
		background->set_useGUILayout(bgTemplate->get_useGUILayout());
		background->set_useLegacyMeshGeneration(bgTemplate->get_useLegacyMeshGeneration());
		background->set_useSpriteMesh(bgTemplate->get_useSpriteMesh());
        
        background->get_transform()->SetParent(parent, false);
        background->set_enabled(true);

        UnityEngine::RectTransform* rectTransform = (UnityEngine::RectTransform*)background->get_transform();
        rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchoredPosition(anchoredPosition);
        rectTransform->set_sizeDelta(sizeDelta);
        
        gameObj->AddComponent<UnityEngine::UI::LayoutElement*>();
        return background;
    }
}