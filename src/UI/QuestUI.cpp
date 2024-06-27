#include "UI/QuestUI.hpp"

#include "GlobalNamespace/UIKeyboardManager.hpp"
#include "GlobalNamespace/SwitchSettingsController.hpp"
#include "GlobalNamespace/ColorPickerButtonController.hpp"
#include "GlobalNamespace/HSVPanelController.hpp"
#include "GlobalNamespace/MenuShockwave.hpp"
#include "GlobalNamespace/LevelCollectionTableView.hpp"

#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/CanvasGroup.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/UI/RectMask2D.hpp"
#include "UnityEngine/UI/ScrollRect.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/Events/UnityAction.hpp"

#include "HMUI/Touchable.hpp"
#include "HMUI/HoverHintController.hpp"
#include "HMUI/TableView.hpp"
#include "HMUI/ImageView.hpp"
#include "HMUI/TextPageScrollView.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "HMUI/TextSegmentedControl.hpp"
#include "HMUI/UIKeyboard.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/EventSystemListener.hpp"
#include "HMUI/DropdownWithTableView.hpp"
#include "HMUI/ButtonSpriteSwap.hpp"
#include "HMUI/TimeSlider.hpp"
#include "HMUI/ColorGradientSlider.hpp"
#include "HMUI/TextSegmentedControl.hpp"
#include "HMUI/HoverTextSetter.hpp"

#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "BGLib/Polyglot/LocalizedTextMeshProUGUI.hpp"

#include "System/Convert.hpp"
#include "System/Action_2.hpp"
#include "System/Action.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"

#include "Libraries/HM/HMLib/VR/HapticPresetSO.hpp"
#include "UI/Components/ExternalComponents.hpp"

#include "Zenject/DiContainer.hpp"

#define DEFAULT_BUTTONTEMPLATE "PracticeButton"

#include "custom-types/shared/delegate.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;
using namespace TMPro;
using namespace HMUI;
using namespace VRUIControls;
using namespace Zenject;
using namespace BGLib::Polyglot;

namespace QuestUI {
    GameObject* beatSaberUIObject = nullptr;
    GameObject* dropdownListPrefab = nullptr;
    GameObject* modalPrefab = nullptr;

    static SafePtrUnity<TMP_FontAsset> mainTextFont;
    TMP_FontAsset* GetMainTextFont() {
        if(!mainTextFont) mainTextFont.emplace(Resources::FindObjectsOfTypeAll<TMP_FontAsset*>()->FirstOrDefault([](TMP_FontAsset* x) { return x->get_name() == "Teko-Medium SDF"; }));
        if(!mainTextFont) {
            return nullptr;
        }
        return mainTextFont.ptr();
    }

    static SafePtrUnity<Material> mainUIFontMaterial;
    Material* GetMainUIFontMaterial() {
        if(!mainUIFontMaterial) mainUIFontMaterial.emplace(Resources::FindObjectsOfTypeAll<Material*>()->FirstOrDefault([](Material* x) { return x->get_name() == "Teko-Medium SDF Curved Softer"; }));
        if(!mainUIFontMaterial) {
            return nullptr;
        }
        return mainUIFontMaterial.ptr();
    }
    
    void SetupPersistentObjects() {
        if(!beatSaberUIObject) {
            static ConstString name("BeatSaberUIObject");
            beatSaberUIObject = GameObject::New_ctor(name);
            GameObject::DontDestroyOnLoad(beatSaberUIObject);
            // beatSaberUIObject->AddComponent<MainThreadScheduler*>();
        }
        if(!dropdownListPrefab) {
            GameObject* search = Resources::FindObjectsOfTypeAll<SimpleTextDropdown*>()->First([](SimpleTextDropdown* x) { 
                    return x->get_transform()->get_parent()->get_name() == "NormalLevels"; 
                }
            )->get_transform()->get_parent()->get_gameObject();
            dropdownListPrefab = Object::Instantiate(search, beatSaberUIObject->get_transform(), false);
            static ConstString name("QuestUIDropdownListPrefab");
            dropdownListPrefab->set_name(name);
            dropdownListPrefab->SetActive(false);
        }
        if (!modalPrefab) {
            GameObject* search = Resources::FindObjectsOfTypeAll<ModalView*>()->First([](ModalView* x) { 
                    return x->get_transform()->get_name() == "DropdownTableView";
                }
            )->get_gameObject();
            modalPrefab = Object::Instantiate(search, beatSaberUIObject->get_transform(), false);
            
            modalPrefab->GetComponent<ModalView*>()->_presentPanelAnimations = search->GetComponent<ModalView*>()->_presentPanelAnimations;
            modalPrefab->GetComponent<ModalView*>()->_dismissPanelAnimation = search->GetComponent<ModalView*>()->_dismissPanelAnimation;

            static ConstString name("QuestUIModalPrefab");
            modalPrefab->set_name(name);
            modalPrefab->SetActive(false);
        }

        GetMainTextFont();
        GetMainUIFontMaterial();
    }

    static PhysicsRaycasterWithCache* physicsRaycaster = nullptr;
    PhysicsRaycasterWithCache* GetPhysicsRaycasterWithCache()
    {
        if(!physicsRaycaster) physicsRaycaster = Resources::FindObjectsOfTypeAll<MainMenuViewController*>()->First()->GetComponent<VRGraphicRaycaster*>()->_physicsRaycaster;
        if(!physicsRaycaster) {
            return nullptr;
        }
        return physicsRaycaster;
    }

    static DiContainer* diContainer = nullptr;
    DiContainer* GetDiContainer()
    {
        if(!diContainer) diContainer = Resources::FindObjectsOfTypeAll<TextSegmentedControl*>()->FirstOrDefault([](TextSegmentedControl* x) { return x->get_transform()->get_parent()->get_name() == "PlayerStatisticsViewController" && x->_container; })->_container;
        if(!diContainer) {
            return nullptr;
        }
        return diContainer;
    }

    static SafePtrUnity<Material> mat_UINoGlow;
    Material* NoGlowMaterial() {
        if(!mat_UINoGlow) mat_UINoGlow.emplace(Resources::FindObjectsOfTypeAll<Material*>()->First([](Material* x) { return x->get_name() == "UINoGlow"; }));
        if(!mat_UINoGlow) {
            return nullptr;
        }

        return mat_UINoGlow.ptr();
    }

    void ClearCache() {
        diContainer = nullptr;
        physicsRaycaster = nullptr;
        beatSaberUIObject = nullptr;
        dropdownListPrefab = nullptr;
        modalPrefab = nullptr;

        mainTextFont = nullptr;
        mainUIFontMaterial = nullptr;
    }

    ModalView* CreateModal(Transform* parent, UnityEngine::Vector2 sizeDelta, UnityEngine::Vector2 anchoredPosition, std::function<void(ModalView*)> onBlockerClicked, bool dismissOnBlockerClicked) {
        static ConstString name("QuestUIModalPrefab");

        // declare var
        ModalView* orig = modalPrefab->GetComponent<ModalView*>();
        
        // instantiate
        GameObject* modalObj = Object::Instantiate(modalPrefab, parent, false);
        
        modalObj->set_name(name);
        modalObj->SetActive(false);

        // get the modal
        ModalView* modal = modalObj->GetComponent<ModalView*>();

        // copy fields
        modal->_presentPanelAnimations = orig->_presentPanelAnimations;
        modal->_dismissPanelAnimation = orig->_dismissPanelAnimation;
        modal->_container = GetDiContainer();
        modalObj->GetComponent<VRGraphicRaycaster*>()->_physicsRaycaster = GetPhysicsRaycasterWithCache();

        // destroy unneeded objects
        Object::DestroyImmediate(modalObj->GetComponent<TableView*>());
        Object::DestroyImmediate(modalObj->GetComponent<ScrollRect*>());
        Object::DestroyImmediate(modalObj->GetComponent<ScrollView*>());
        Object::DestroyImmediate(modalObj->GetComponent<EventSystemListener*>());

        // destroy all children except background
        int childCount = modal->get_transform()->get_childCount();
        for (int i = 0; i < childCount; i++) {
            auto* child = modal->get_transform()->GetChild(i)->GetComponent<RectTransform*>();

            if (child->get_gameObject()->get_name() == "BG") {
                child->set_anchoredPosition(Vector2::get_zero());
                child->set_sizeDelta(Vector2::get_zero());
                child->GetComponent<Image*>()->set_raycastTarget(true);
            }
            else {
                // yeet the child
                Object::Destroy(child->get_gameObject());
            }
        }

        // set recttransform data
        auto rect = modalObj->GetComponent<RectTransform*>();
        rect->set_anchorMin({0.5f, 0.5f});
        rect->set_anchorMax({0.5f, 0.5f});
        rect->set_sizeDelta(sizeDelta);
        rect->set_anchoredPosition(anchoredPosition);

        // add callback
        modal->add_blockerClickedEvent(
            custom_types::MakeDelegate<System::Action *>(classof(System::Action *), (std::function<void()>) [onBlockerClicked, modal, dismissOnBlockerClicked] () {
                if (onBlockerClicked)
                    onBlockerClicked(modal); 
                if (dismissOnBlockerClicked)
                    modal->Hide(true, nullptr);
            })
        );
        return modal;
    }

    TextMeshProUGUI* CreateText(Transform* parent, StringW text, UnityEngine::Vector2 anchoredPosition) {
        return CreateText(parent, text, true, anchoredPosition);
    }

    TextMeshProUGUI* CreateText(Transform* parent, StringW text, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
        return CreateText(parent, text, true, anchoredPosition, sizeDelta);
    }

    TextMeshProUGUI* CreateText(Transform* parent, StringW text, bool italic) {
        return CreateText(parent, text, italic, UnityEngine::Vector2(0.0f, 0.0f), UnityEngine::Vector2(60.0f, 10.0f));
    }

    TextMeshProUGUI* CreateText(Transform* parent, StringW text, bool italic, UnityEngine::Vector2 anchoredPosition) {
        return CreateText(parent, text, italic, anchoredPosition, UnityEngine::Vector2(60.0f, 10.0f));
    }

    TextMeshProUGUI* CreateText(Transform* parent, StringW text, bool italic, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
        static ConstString name("QuestUIText");
        GameObject* gameObj = GameObject::New_ctor(name);
        gameObj->SetActive(false);

        CurvedTextMeshPro* textMesh = gameObj->AddComponent<CurvedTextMeshPro*>();
        RectTransform* rectTransform = textMesh->get_rectTransform();
        rectTransform->SetParent(parent, false);
        textMesh->set_font(GetMainTextFont());
        textMesh->set_fontSharedMaterial(GetMainUIFontMaterial());
        if (italic) textMesh->set_fontStyle(TMPro::FontStyles::Italic);
        textMesh->set_text(text);
        textMesh->set_fontSize(4.0f);
        textMesh->set_color(UnityEngine::Color::get_white());
        textMesh->set_richText(true);
        rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchoredPosition(anchoredPosition);
        rectTransform->set_sizeDelta(sizeDelta);
        
        gameObj->AddComponent<LayoutElement*>();

        gameObj->SetActive(true);
        return textMesh;
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, std::string_view buttonTemplate, std::function<void()> onClick) {
        static std::unordered_map<std::string, SafePtrUnity<Button>> buttonCopyMap;
        auto& buttonCopy = buttonCopyMap[std::string(buttonTemplate)];
        if (!buttonCopy) {
            buttonCopy = Resources::FindObjectsOfTypeAll<Button*>()->LastOrDefault([&buttonTemplate](auto* x) { return x->get_name() == buttonTemplate; });
        }

        Button* button = Object::Instantiate(buttonCopy.ptr(), parent, false);
        button->set_onClick(Button::ButtonClickedEvent::New_ctor());
        static ConstString name("QuestUIButton");
        button->set_name(name);
        if(onClick)
            button->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(onClick));

        LocalizedTextMeshProUGUI* localizer = button->GetComponentInChildren<LocalizedTextMeshProUGUI*>();
        if (localizer != nullptr)
            GameObject::Destroy(localizer);
        ExternalComponents* externalComponents = button->get_gameObject()->AddComponent<ExternalComponents*>();

        TextMeshProUGUI* textMesh = button->GetComponentInChildren<TextMeshProUGUI*>();
        if (textMesh)
        {
            textMesh->set_richText(true);
            textMesh->set_alignment(TextAlignmentOptions::Center);
            textMesh->set_text(buttonText);
            externalComponents->Add(textMesh);
        }
        RectTransform* rectTransform = button->get_transform().cast<RectTransform>();
        rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_pivot(UnityEngine::Vector2(0.5f, 0.5f));

        HorizontalLayoutGroup* horiztonalLayoutGroup = button->GetComponentInChildren<HorizontalLayoutGroup*>();
        if (horiztonalLayoutGroup != nullptr)
            externalComponents->Add(horiztonalLayoutGroup);
        
        // if the original button was for some reason not interactable, now it will be
        button->set_interactable(true);
        button->get_gameObject()->SetActive(true);
        return button;
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, std::string_view buttonTemplate, UnityEngine::Vector2 anchoredPosition, std::function<void()> onClick) {
        Button* button = CreateUIButton(parent, buttonText, buttonTemplate, onClick);
        button->GetComponent<RectTransform*>()->set_anchoredPosition(anchoredPosition);
        return button;
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, std::string_view buttonTemplate, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick) {
        Button* button = CreateUIButton(parent, buttonText, buttonTemplate, anchoredPosition, onClick);
        button->GetComponent<RectTransform*>()->set_sizeDelta(sizeDelta);
        LayoutElement* layoutElement = button->GetComponent<LayoutElement*>();
        if(!layoutElement)
            layoutElement = button->get_gameObject()->AddComponent<LayoutElement*>();
        layoutElement->set_minWidth(sizeDelta.x);
        layoutElement->set_minHeight(sizeDelta.y);
        layoutElement->set_preferredWidth(sizeDelta.x);
        layoutElement->set_preferredHeight(sizeDelta.y);
        layoutElement->set_flexibleWidth(sizeDelta.x);
        layoutElement->set_flexibleHeight(sizeDelta.y);
        return button;
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, std::function<void()> onClick) {
        return CreateUIButton(parent, buttonText, DEFAULT_BUTTONTEMPLATE, onClick);
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, UnityEngine::Vector2 anchoredPosition, std::function<void()> onClick) {
        return CreateUIButton(parent, buttonText, DEFAULT_BUTTONTEMPLATE, anchoredPosition, onClick);
    }

    Button* CreateUIButton(Transform* parent, StringW buttonText, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick) {
        return CreateUIButton(parent, buttonText, DEFAULT_BUTTONTEMPLATE, anchoredPosition, sizeDelta, onClick);
    }

    ImageView* CreateImage(Transform* parent, Sprite* sprite, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta) {
        static ConstString name("QuestUIImage");
        GameObject* gameObj = GameObject::New_ctor(name);
        ImageView* image = gameObj->AddComponent<ImageView*>();
        image->set_material(NoGlowMaterial());
        image->get_transform()->SetParent(parent, false);
        image->set_sprite(sprite);
        RectTransform* rectTransform = image->get_transform().cast<UnityEngine::RectTransform>();
        rectTransform->set_anchorMin(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchorMax(UnityEngine::Vector2(0.5f, 0.5f));
        rectTransform->set_anchoredPosition(anchoredPosition);
        rectTransform->set_sizeDelta(sizeDelta);
        
        gameObj->AddComponent<LayoutElement*>();
        return image;
    }
}