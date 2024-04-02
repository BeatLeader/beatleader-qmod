#include "System/Reflection/BindingFlags.hpp"
#include "System/Reflection/PropertyInfo.hpp"
#include "System/Reflection/FieldInfo.hpp"
#include "System/Type.hpp"
#include "System/Object.hpp"

#include "UI/UIUtils.hpp"

#include "include/Models/TriangleRating.hpp"
#include "include/Utils/ModConfig.hpp"

#include "main.hpp"

#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Resources.hpp"

namespace UIUtils {

    SafePtrUnity<HMUI::ImageView> roundRectSprite;

    HMUI::ImageView* getRoundRectSprite() {
        if (!roundRectSprite) {
            roundRectSprite = UnityEngine::Resources::FindObjectsOfTypeAll<HMUI::ImageView*>()->First([](HMUI::ImageView* image){ 
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
        // Get all of the mod settings infos, and get the one that is for beatleader

        // TODO Is this even possible anymore with BSML? This accessed internal structures of questui ...
        // for (auto& s : BSML::ModSettingsInfos::get()) {
        //     if (s.modInfo.id == MOD_ID) {
        //         s.Present();
        //     }
        // }
    }

    void AddRoundRect(HMUI::ImageView* background) {
        auto bgTemplate = getRoundRectSprite();
        background->set_alphaHitTestMinimumThreshold(bgTemplate->get_alphaHitTestMinimumThreshold());
		background->set_color(bgTemplate->get_color());
		background->set_color0(bgTemplate->get_color0());
		background->set_color1(bgTemplate->get_color1());
		background->set_gradient(bgTemplate->get_gradient());
		background->_gradientDirection = bgTemplate->_gradientDirection;
        background->_flipGradientColors = bgTemplate->_flipGradientColors;
        background->_skew = bgTemplate->_skew;
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
    }

    // Copied from BSML
    HMUI::ImageView* CreateRoundRectImage(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
        static ConstString name("bsmlImage");
        UnityEngine::GameObject* gameObj = UnityEngine::GameObject::New_ctor(name);
        HMUI::ImageView* background = gameObj->AddComponent<HMUI::ImageView*>();// GetCopyOf(, getRoundRectSprite());
        AddRoundRect(background);
        
        background->get_transform()->SetParent(parent, false);
        background->set_enabled(true);

        UnityEngine::RectTransform* rectTransform = background->get_transform().cast<UnityEngine::RectTransform>();
        rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchoredPosition(anchoredPosition);
        rectTransform->set_sizeDelta(sizeDelta);
        
        gameObj->AddComponent<UnityEngine::UI::LayoutElement*>();
        return background;
    }

    float getStarsToShow(TriangleRating rating){
        switch(getModConfig().StarValueToShow.GetValue()){
            case 1:
                return rating.techRating;
            case 2:
                return rating.accRating;
            case 3:
                return rating.passRating;
            default:
                return rating.stars;
        }
    }
}