#pragma once

#include "bsml/shared/BSML/TypeHandlers/TypeHandler.hpp"
#include "UI/BSML_Addons/Components/BetterImage.hpp"
#include "UnityEngine/UI/Image.hpp"

namespace BeatLeader::UI::TypeHandlers {
    class BetterImageHandler : public BSML::TypeHandler<BeatLeader::UI::BSML_Addons::BetterImage*> {
        using Base = TypeHandler<BeatLeader::UI::BSML_Addons::BetterImage*>;
        using Base::Base;

        virtual Base::PropMap get_props() const override;
        virtual Base::SetterMap get_setters() const override;
        virtual void HandleType(const BSML::ComponentTypeWithData& componentType, BSML::BSMLParserParams& parserParams) override;

    public:
        static void HandleImage(const std::map<std::string, std::string>& data, UnityEngine::UI::Image* image);
        static void HandleImage(const std::map<std::string, std::string>& data, UnityEngine::UI::Image* image, const std::string& prefix);
    };
} 