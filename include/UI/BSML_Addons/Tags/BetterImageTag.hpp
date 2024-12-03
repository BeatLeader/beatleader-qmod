#pragma once

#include "bsml/shared/BSML/Tags/BSMLTag.hpp"

namespace BeatLeader::UI::BSML_Addons {
    class BetterImageTag : public BSML::BSMLTag {
        public:
            using BSML::BSMLTag::BSMLTag;
        protected:
            UnityEngine::GameObject* CreateObject(UnityEngine::Transform* parent) const override;
    };
} 