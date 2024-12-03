#include "UI/BSML_Addons/Tags/BetterImageTag.hpp"
#include "UI/BSML_Addons/Components/BetterImage.hpp"
#include "UI/BSML_Addons/Components/FixedImageView.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "bsml/shared/BSML/Components/ExternalComponents.hpp"
#include "bsml/shared/BSML/Components/Backgroundable.hpp"

namespace BeatLeader::UI::BSML_Addons {
    static BSML::BSMLNodeParser<BetterImageTag> carouselTagParser({"better-image", "better-img", "custom-image", "custom-img"});

    UnityEngine::GameObject* BetterImageTag::CreateObject(UnityEngine::Transform* parent) const {
        auto* gameObject = UnityEngine::GameObject::New_ctor("BeatLeaderBetterImage");

        auto* image = gameObject->AddComponent<FixedImageView*>();
        image->get_rectTransform()->set_sizeDelta(UnityEngine::Vector2(20.0f, 20.0f));
        image->set_sprite(BSML::Utilities::ImageResources::GetBlankSprite());

        auto* fitter = gameObject->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
        gameObject->AddComponent<UnityEngine::UI::LayoutElement*>();
        
        auto* betterImage = gameObject->AddComponent<BetterImage*>();
        betterImage->image = image;

        image->get_rectTransform()->SetParent(parent, false);
        return gameObject;
    }
} 