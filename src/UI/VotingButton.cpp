#include "HMUI/Touchable.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "UnityEngine/Application.hpp"
#include "UnityEngine/GUIUtility.hpp"

#include "Assets/BundleLoader.hpp"

#include "include/UI/VotingButton.hpp"
#include "include/Utils/StringUtils.hpp"

#include "main.hpp"

#include <string>
#include <cmath>

using namespace QuestUI;
using namespace std;

DEFINE_TYPE(BeatLeader, VotingButton);

static int SpinnerValuePropertyId;
static int GradientValuePropertyId;
static int StatePropertyId;
static int TintPropertyId;

void BeatLeader::VotingButton::Init(QuestUI::ClickableImage* imageView) {
    this->imageView = imageView;

    this->materialInstance = UnityEngine::Object::Instantiate(BundleLoader::bundle->VotingButtonMaterial);
    imageView->set_material(this->materialInstance);

    imageView->set_defaultColor(UnityEngine::Color(0.0, 0.0, 0.0, 1.0));
    imageView->set_highlightColor(UnityEngine::Color(1.0, 0.0, 0.0, 1.0));

    this->hoverHint = QuestUI::BeatSaberUI::AddHoverHint(imageView->get_gameObject(), "Rank voting");

    SpinnerValuePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_SpinnerValue"));
    GradientValuePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_GradientValue"));
    StatePropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_State"));
    TintPropertyId = UnityEngine::Shader::PropertyToID(newcsstr2("_Tint"));
}

void UpdateMaterial(UnityEngine::Material* materialInstance, float spinnerValue, float gradientValue, float state, UnityEngine::Color const& color) {
    materialInstance->SetFloat(SpinnerValuePropertyId, spinnerValue);
    materialInstance->SetFloat(GradientValuePropertyId, gradientValue);
    materialInstance->SetFloat(StatePropertyId, state);
    materialInstance->SetColor(TintPropertyId, color);
}

void BeatLeader::VotingButton::SetState(int state) {
    this->state = state;
    switch (state)
    {
    case -1:
        UpdateMaterial(materialInstance, 0, 0, 0, UnityEngine::Color(0.2f, 0.2f, 0.2f, 0.0f));
        hoverHint->set_text(newcsstr2("Voting for this map is not supported"));
        break;
    case 0:
        UpdateMaterial(materialInstance, 1, 0, 0, UnityEngine::Color(0.2f, 0.2f, 0.2f, 0.0f));
        hoverHint->set_text(newcsstr2("Loading vote status"));
        break;
    case 1:
        UpdateMaterial(materialInstance, 0, 0, 0, UnityEngine::Color(0.2f, 0.2f, 0.2f, 0.0f));
        hoverHint->set_text(newcsstr2("Pass this map to vote it for ranked"));
        break;
    case 2:
        UpdateMaterial(materialInstance, 0, 1, 1, UnityEngine::Color(1.0f, 1.0f, 1.0f, 0.7f));
        hoverHint->set_text(newcsstr2("Vote for this map ranking status and stars"));
        break;
    case 3:
        UpdateMaterial(materialInstance, 0, 0, 2, UnityEngine::Color(0.2f, 0.8f, 0.2f, 0.4f));
        hoverHint->set_text(newcsstr2("Thank you for the vote!"));
        break;
    
    default:
        break;
    }
}