#pragma once

#include "bsml/shared/BSML/Tags/BSMLTag.hpp"

namespace BeatLeader::UI::BSML_Addons {
    class BetterButtonTag : public BSML::BSMLTag {
        public:
            BetterButtonTag() : BSMLTag() {}
        protected:
            UnityEngine::GameObject* CreateObject(UnityEngine::Transform* parent) const override;
    };
} 