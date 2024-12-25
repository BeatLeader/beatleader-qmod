#include "UI/Components/StaticScreen.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/RectTransform.hpp"

#include "Zenject/DiContainer.hpp"
#include "BSML/shared/Helpers/getters.hpp"

DEFINE_TYPE(BeatLeader, StaticScreen);

namespace BeatLeader {

    void StaticScreen::Present() {
        targetAlpha = 1.0f;
        set = false;
        get_gameObject()->SetActive(true);
    }

    void StaticScreen::Dismiss() {
        targetAlpha = 0.0f;
        set = false;
    }

    void StaticScreen::Update() {
        if (set) return;

        float val = UnityEngine::Mathf::Lerp(canvasGroup->get_alpha(), targetAlpha, UnityEngine::Time::get_deltaTime() * 10.0f);
        if (UnityEngine::Mathf::Abs(targetAlpha - val) < 0.001f) {
            val = targetAlpha;
            set = true;
            get_gameObject()->SetActive(UnityEngine::Mathf::Approximately(val, 1.0f));
        }
        canvasGroup->set_alpha(val);
    }

    void StaticScreen::Awake() {
        // Setup Canvas
        auto canvas = get_gameObject()->AddComponent<UnityEngine::Canvas*>();
        canvas->set_referencePixelsPerUnit(10.0f);
        canvas->set_additionalShaderChannels(
            (UnityEngine::AdditionalCanvasShaderChannels)((int)UnityEngine::AdditionalCanvasShaderChannels::TexCoord1 | 
            (int)UnityEngine::AdditionalCanvasShaderChannels::TexCoord2));
        canvas->set_sortingOrder(5);

        // Add HMUI Screen
        get_gameObject()->AddComponent<HMUI::Screen*>();

        // Setup CanvasGroup
        canvasGroup = get_gameObject()->AddComponent<UnityEngine::CanvasGroup*>();
        canvasGroup->set_alpha(0.0f);
        get_gameObject()->SetActive(false);

        // Setup Raycaster
        auto raycaster = get_gameObject()->AddComponent<VRUIControls::VRGraphicRaycaster*>();
        BSML::Helpers::GetDiContainer()->Inject(raycaster);

        // Set scale
        get_transform()->set_localScale(UnityEngine::Vector3(0.02f, 0.02f, 0.02f));
    }

} // namespace BeatLeader 