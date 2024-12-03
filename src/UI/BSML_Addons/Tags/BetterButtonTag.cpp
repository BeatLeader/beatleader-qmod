#include "UI/BSML_Addons/Tags/BetterButtonTag.hpp"
#include "UI/BSML_Addons/Components/BetterButton.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "bsml/shared/BSML/Components/ExternalComponents.hpp"
#include "bsml/shared/BSML/Components/Backgroundable.hpp"

namespace BeatLeader::UI::BSML_Addons {
    static BSML::BSMLNodeParser<BetterButtonTag> betterButtonTagParser({"better-button"});

    UnityEngine::GameObject* BetterButtonTag::CreateObject(UnityEngine::Transform* parent) const {
        auto* container = UnityEngine::GameObject::New_ctor("BeatLeaderBetterButton");

        auto* unityButton = container->AddComponent<BetterButton*>();
        auto* image = container->AddComponent<UnityEngine::UI::Image*>();
        auto* button = container->AddComponent<UnityEngine::UI::Button*>();
        auto* contentSizeFitter = container->AddComponent<UnityEngine::UI::ContentSizeFitter*>();
        container->AddComponent<UnityEngine::UI::LayoutElement*>();

        unityButton->Init(button, image);
        image->get_rectTransform()->set_sizeDelta(UnityEngine::Vector2(20.0f, 20.0f));
        image->set_sprite(BSML::Utilities::ImageResources::GetBlankSprite());
        
        contentSizeFitter->set_verticalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
        contentSizeFitter->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);

        container->get_transform()->SetParent(parent, false);

        return container;
    }
} 