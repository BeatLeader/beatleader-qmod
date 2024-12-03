#include "UI/BSML_Addons/BetterImageHandler.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "Utils/BSMLUtility.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/FormatUtils.hpp"

#include "main.hpp"

namespace BeatLeader::UI::TypeHandlers {
    static BetterImageHandler betterImageHandler{};

    BetterImageHandler::Base::PropMap BetterImageHandler::get_props() const {
        return {
            { "image", { "source" , "src" } },
            { "preserveAspect", { "preserve-aspect" } },
            { "raycastImage", { "raycast-image" } },
            { "imageColor", { "image-color", "img-color", "color" } },
            { "imageType", { "image-type", "type" } },
            { "fillMethod", { "fill-method" } },
            { "fillOrigin", { "fill-origin" } },
            { "fillAmount", { "fill-amount" } },
            { "fillClockwise", { "fill-clockwise" } },
            { "pixelsPerUnit", { "pixels-per-unit-multiplier", "pixels-per-unit", "ppu" } }
        };
    }

    BetterImageHandler::Base::SetterMap BetterImageHandler::get_setters() const {
        return {};
    }

    void BetterImageHandler::HandleType(const BSML::ComponentTypeWithData& componentType, BSML::BSMLParserParams& parserParams) {
        Base::HandleType(componentType, parserParams);
        
        auto image = reinterpret_cast<BeatLeader::UI::BSML_Addons::BetterImage*>(componentType.component);
        HandleImage(componentType.data, image->image);
    }

    void BetterImageHandler::HandleImage(const std::map<std::string, std::string>& data, UnityEngine::UI::Image* image) {
        try {
            auto imagePathItr = data.find("image");
            if (imagePathItr != data.end()) {
                image->set_sprite(Utils::BSMLUtility::LoadSprite(imagePathItr->second));
            }

            auto imageColorItr = data.find("imageColor");
            if (imageColorItr != data.end()) {
                image->set_color(FormatUtils::ParseColor(imageColorItr->second));
            }

            auto preserveAspectItr = data.find("preserveAspect");
            if (preserveAspectItr != data.end()) {
                image->set_preserveAspect(FormatUtils::ParseBool(preserveAspectItr->second));
            }

            auto raycastImageItr = data.find("raycastImage");
            if (raycastImageItr != data.end()) {
                image->set_raycastTarget(FormatUtils::ParseBool(raycastImageItr->second));
            }

            auto imageTypeItr = data.find("imageType");
            if (imageTypeItr != data.end()) {
                image->set_type(FormatUtils::ParseEnum<UnityEngine::UI::Image::Type>(imageTypeItr->second));
            }

            auto fillMethodItr = data.find("fillMethod");
            if (fillMethodItr != data.end()) {
                image->set_fillMethod(FormatUtils::ParseEnum<UnityEngine::UI::Image::FillMethod>(fillMethodItr->second));
            }

            auto fillOriginItr = data.find("fillOrigin");
            if (fillOriginItr != data.end()) {
                int origin = FormatUtils::ParseInt(fillOriginItr->second);
                if (origin >= 0) {
                    image->set_fillOrigin(origin);
                }
            }

            auto fillAmountItr = data.find("fillAmount");
            if (fillAmountItr != data.end()) {
                image->set_fillAmount(FormatUtils::ParseFloat(fillAmountItr->second));
            }

            auto fillClockwiseItr = data.find("fillClockwise");
            if (fillClockwiseItr != data.end()) {
                image->set_fillClockwise(FormatUtils::ParseBool(fillClockwiseItr->second));
            }

            auto ppuItr = data.find("pixelsPerUnit");
            if (ppuItr != data.end()) {
                image->set_pixelsPerUnitMultiplier(FormatUtils::ParseFloat(ppuItr->second));
            }

        } catch (const std::exception& e) {
            BeatLeaderLogger.error("BeatLeader BetterImageHandler: {}", e.what());
        }
    }

    void BetterImageHandler::HandleImage(const std::map<std::string, std::string>& data, UnityEngine::UI::Image* image, const std::string& prefix) {
        std::map<std::string, std::string> generatedData;
        for (const auto& [key, value] : data) {
            std::string newKey = key;
            if (key.find(prefix) != std::string::npos) {
                newKey = key.substr(prefix.length());
                newKey[0] = std::tolower(newKey[0]);
            }
            generatedData[newKey] = value;
        }
        HandleImage(generatedData, image);
    }
} 