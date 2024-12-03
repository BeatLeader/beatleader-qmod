#include "UI/BSML_Addons/BetterButtonHandler.hpp"
#include "UI/BSML_Addons/BetterImageHandler.hpp"
#include "UnityEngine/UI/Selectable.hpp"
#include "UnityEngine/UI/ColorBlock.hpp"
#include "UnityEngine/UI/SpriteState.hpp"
#include "Utils/BSMLUtility.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/FormatUtils.hpp"

#include "main.hpp"

namespace BeatLeader::UI::TypeHandlers {
    static BetterButtonHandler betterButtonHandler{};

    BetterButtonHandler::Base::PropMap BetterButtonHandler::get_props() const {
        return {
            { "onClick", { "on-click" } },
            { "clickEvent", { "click-event", "event-click"} },
            { "transition", { "transition" } },
            { "normalColor", { "normal-color" } },
            { "highlightedColor", { "highlighted-color" } },
            { "pressedColor", { "pressed-color" } },
            { "colorMultiplier", { "color-multiplier" } },
            { "highlightedSprite", { "highlighted-sprite", "highlighted-image" } },
            { "pressedSprite", { "pressed-sprite" , "pressed-image" } },
            { "image", { "normal-sprite", "normal-image" } },
            { "preserveAspect", { "preserve-aspect" } },
            { "imageColor", { "image-color", "img-color" } },
            { "imageType", { "image-type", "type" } },
            { "fillMethod", { "fill-method" } },
            { "fillOrigin", { "fill-origin" } },
            { "fillAmount", { "fill-amount" } },
            { "fillClockwise", { "fill-clockwise", "clockwise" } },
            { "pixelsPerUnit", { "pixels-per-unit-multiplier", "pixels-per-unit", "ppu" } }
        };
    }

    BetterButtonHandler::Base::SetterMap BetterButtonHandler::get_setters() const {
        return {};
    }

    void BetterButtonHandler::HandleType(const BSML::ComponentTypeWithData& componentType, BSML::BSMLParserParams& parserParams) {
        Base::HandleType(componentType, parserParams);
        
        auto button = reinterpret_cast<BeatLeader::UI::BSML_Addons::BetterButton*>(componentType.component);
        auto& data = componentType.data;

        if (!button) return;

        try {
            auto transitionItr = data.find("transition");
            if (transitionItr != data.end() && transitionItr->second != "Animation") {
                button->button->set_transition(FormatUtils::ParseEnum<UnityEngine::UI::Selectable::Transition>(transitionItr->second));
            }

            // Handle colors
            auto colors = button->button->get_colors();
            
            auto normalColorItr = data.find("normalColor");
            if (normalColorItr != data.end()) {
                colors.normalColor = FormatUtils::ParseColor(normalColorItr->second);
            }

            auto highlightedColorItr = data.find("highlightedColor");
            if (highlightedColorItr != data.end()) {
                colors.highlightedColor = FormatUtils::ParseColor(highlightedColorItr->second);
            }

            auto pressedColorItr = data.find("pressedColor");
            if (pressedColorItr != data.end()) {
                colors.pressedColor = FormatUtils::ParseColor(pressedColorItr->second);
            }

            auto colorMultiplierItr = data.find("colorMultiplier");
            if (colorMultiplierItr != data.end()) {
                colors.colorMultiplier = FormatUtils::ParseFloat(colorMultiplierItr->second);
            }

            button->button->set_colors(colors);

            // Handle sprites
            auto spriteState = button->button->get_spriteState();
            
            auto highlightedSpriteItr = data.find("highlightedSprite");
            if (highlightedSpriteItr != data.end()) {
                spriteState.highlightedSprite = Utils::BSMLUtility::LoadSprite(highlightedSpriteItr->second);
            }

            auto pressedSpriteItr = data.find("pressedSprite");
            if (pressedSpriteItr != data.end()) {
                spriteState.pressedSprite = Utils::BSMLUtility::LoadSprite(pressedSpriteItr->second);
            }

            button->button->set_spriteState(spriteState);

            // Handle image properties through BetterImageHandler
            BetterImageHandler::HandleImage(data, button->targetGraphic);

        } catch (const std::exception& e) {
            BeatLeaderLogger.error("BeatLeader BetterButtonHandler: {}", e.what());
        }
    }
} 