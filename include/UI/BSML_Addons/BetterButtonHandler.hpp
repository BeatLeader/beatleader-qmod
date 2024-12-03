#pragma once

#include "bsml/shared/BSML/TypeHandlers/TypeHandler.hpp"
#include "UI/BSML_Addons/Components/BetterButton.hpp"

namespace BeatLeader::UI::TypeHandlers {
    class BetterButtonHandler : public BSML::TypeHandler<BeatLeader::UI::BSML_Addons::BetterButton*> {
        using Base = TypeHandler<BeatLeader::UI::BSML_Addons::BetterButton*>;
        using Base::Base;

        virtual Base::PropMap get_props() const override;
        virtual Base::SetterMap get_setters() const override;
        virtual void HandleType(const BSML::ComponentTypeWithData& componentType, BSML::BSMLParserParams& parserParams) override;
    };
} 