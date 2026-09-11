#include "UI/BSML_Addons/BetterImageHandler.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "Utils/BSMLUtility.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/FormatUtils.hpp"

#include "main.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <utility>

namespace BeatLeader::UI::TypeHandlers {
    namespace {
        std::string ToLower(std::string value) {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
            return value;
        }

        // BSML attributes carry enum names (e.g. type="Sliced"); accept names case-insensitively or a numeric value
        template<typename T>
        T ParseNamedEnum(std::string const& value, std::initializer_list<std::pair<char const*, T>> names) {
            auto lowered = ToLower(value);
            for (auto const& [name, enumValue] : names) {
                if (lowered == name) return enumValue;
            }
            return static_cast<T>(std::stoi(value));
        }

        UnityEngine::UI::Image::Type ParseImageType(std::string const& value) {
            using Type = UnityEngine::UI::Image::Type;
            return ParseNamedEnum<Type>(value, {{"simple", Type::Simple}, {"sliced", Type::Sliced}, {"tiled", Type::Tiled}, {"filled", Type::Filled}});
        }

        UnityEngine::UI::Image::FillMethod ParseFillMethod(std::string const& value) {
            using FillMethod = UnityEngine::UI::Image::FillMethod;
            return ParseNamedEnum<FillMethod>(value, {{"horizontal", FillMethod::Horizontal}, {"vertical", FillMethod::Vertical}, {"radial90", FillMethod::Radial90}, {"radial180", FillMethod::Radial180}, {"radial360", FillMethod::Radial360}});
        }
    }

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
                image->set_type(ParseImageType(imageTypeItr->second));
            }

            auto fillMethodItr = data.find("fillMethod");
            if (fillMethodItr != data.end()) {
                image->set_fillMethod(ParseFillMethod(fillMethodItr->second));
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