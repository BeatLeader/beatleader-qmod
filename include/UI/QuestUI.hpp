#pragma once
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/utils-functions.h"

#include "GlobalNamespace/MainFlowCoordinator.hpp"
#include "GlobalNamespace/GameplayModifierToggle.hpp"
#include "GlobalNamespace/ColorChangeUIEventType.hpp"
#include "GlobalNamespace/IVRPlatformHelper.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/UI/GridLayoutGroup.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/HoverHint.hpp"
#include "HMUI/InputFieldView.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/ImageView.hpp"
#include "HMUI/SimpleTextDropdown.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "TMPro/TMP_FontAsset.hpp"

#include "UI/Components/CustomTextSegmentedControlData.hpp"

#include <concepts>
#include <type_traits>

namespace QuestUI {
    void SetupPersistentObjects();
    void ClearCache();

    /// @brief creates a modal that can be used to display information
    /// @param parent what to parent it to
    /// @param onBlockerClicked callback that gets called when clicking next to the modal, leaving it empty makes it just dismiss the modal
    /// @param sizeDelta size of the object
    /// @param anchoredPosition position of the modal
    /// @param dismissOnBlockerClicked whether to auto dismiss when the blocker (outside) is clicked
    /// @return created modal
    HMUI::ModalView* CreateModal(UnityEngine::Transform* parent, UnityEngine::Vector2 sizeDelta, UnityEngine::Vector2 anchoredPosition, std::function<void(HMUI::ModalView*)> onBlockerClicked, bool dismissOnBlockerClicked = true);

    /// @brief Creates a text object
    /// @param parent what to parent it to
    /// @param text the string to display
    /// @param italic should the text be italic
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::Transform* parent, StringW text, bool italic = true);

    /// @brief Creates a text object
    /// @param parent what to parent it to
    /// @param text the string to display
    /// @param anchoredPosition the location of the text
    /// @param sizeDelta the size of the text
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::Transform* parent, StringW text, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta);

    /// @brief Creates a text object
    /// @param parent what to parent it to
    /// @param text the string to display
    /// @param anchoredPosition the location of the text
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::Transform* parent, StringW text, UnityEngine::Vector2 anchoredPosition);

    /// @brief Creates a text object
    /// @param parent what to parent it to
    /// @param text the string to display
    /// @param italic should the text be italic
    /// @param anchoredPosition the location of the text
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::Transform* parent, StringW text, bool italic, UnityEngine::Vector2 anchoredPosition);

    /// @brief Creates a text object
    /// @param parent what to parent it to
    /// @param text the string to display
    /// @param italicc should the text be italic
    /// @param anchoredPosition the location of the text
    /// @param sizeDelta the size of the text
    TMPro::TextMeshProUGUI* CreateText(UnityEngine::Transform* parent, StringW text, bool italic, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, std::function<void()> onClick = nullptr);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param buttonTemplate the name of a button to use as a base
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, std::string_view buttonTemplate, std::function<void()> onClick = nullptr);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param buttonTemplate the name of a button to use as a base
    /// @param anchoredPosition position of the button
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, std::string_view buttonTemplate, UnityEngine::Vector2 anchoredPosition, std::function<void()> onClick = nullptr);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param buttonTemplate the name of a button to use as a base
    /// @param anchoredPosition position of the button
    /// @param sizeDelta size of the button
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, std::string_view buttonTemplate, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick = nullptr);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param anchoredPosition position of the button
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, UnityEngine::Vector2 anchoredPosition, std::function<void()> onClick = nullptr);

    /// @brief Creates a button to click and have an action happen
    /// @param parent what to parent it to
    /// @param buttonText the text the butotn displays
    /// @param anchoredPosition position of the button
    /// @param sizeDelta size of the button
    /// @param onClick callback to run when the button is clicked
    UnityEngine::UI::Button* CreateUIButton(UnityEngine::Transform* parent, StringW buttonText, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, std::function<void()> onClick = nullptr);

    /// @brief Creates an image
    /// @param parent what to parent it to
    /// @param sprite the sprite to display
    /// @param anchoredPosition the position of the image
    /// @param sizeDelta the size of the image
    HMUI::ImageView* CreateImage(UnityEngine::Transform* parent, UnityEngine::Sprite* sprite, UnityEngine::Vector2 anchoredPosition = {}, UnityEngine::Vector2 sizeDelta = {});

    /// @brief creates a text segmented control like the one on the gameplay setup view controller
    /// @param parent what to parent it to
    /// @param anchoredPosition the position
    /// @param sizeDelta the sizeDelta
    /// @param values list of text values to give to the controller
    /// @param onCellWithIdxClicked callback called when a cell is clicked
    /// @return the created text segmented control
    QuestUI::CustomTextSegmentedControlData* CreateTextSegmentedControl(UnityEngine::Transform* parent, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, ArrayW<StringW> values, std::function<void(int)> onCellWithIdxClicked = nullptr);
    
    /// @brief creates a text segmented control like the one on the gameplay setup view controller
    /// @param parent what to parent it to
    /// @param sizeDelta the sizeDelta
    /// @param values list of text values to give to the controller
    /// @param onCellWithIdxClicked callback called when a cell is clicked
    /// @return the created text segmented control
    QuestUI::CustomTextSegmentedControlData* CreateTextSegmentedControl(UnityEngine::Transform* parent, UnityEngine::Vector2 sizeDelta, ArrayW<StringW> values, std::function<void(int)> onCellWithIdxClicked = nullptr);
    
    /// @brief creates a text segmented control like the one on the gameplay setup view controller
    /// @param parent what to parent it to
    /// @param values list of text values to give to the controller
    /// @param onCellWithIdxClicked callback called when a cell is clicked
    /// @return the created text segmented control
    QuestUI::CustomTextSegmentedControlData* CreateTextSegmentedControl(UnityEngine::Transform* parent, ArrayW<StringW> values, std::function<void(int)> onCellWithIdxClicked);

    /// @brief creates a text segmented control like the one on the gameplay setup view controller
    /// @param parent what to parent it to
    /// @param onCellWithIdxClicked callback called when a cell is clicked
    /// @return the created text segmented control
    QuestUI::CustomTextSegmentedControlData* CreateTextSegmentedControl(UnityEngine::Transform* parent, std::function<void(int)> onCellWithIdxClicked);
}