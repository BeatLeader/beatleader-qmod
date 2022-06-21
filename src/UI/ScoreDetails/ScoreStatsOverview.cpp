#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ScoreDetails/ScoreStatsOverview.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

static UnityEngine::Color LeftColor = UnityEngine::Color(0.8f, 0.2f, 0.2f, 0.1f);
static UnityEngine::Color RightColor = UnityEngine::Color(0.2f, 0.2f, 0.8f, 0.1f);
static int FillPropertyId = 0;

BeatLeader::ScoreStatsOverview::ScoreStatsOverview(HMUI::ModalView *modal) noexcept {

    leftPreScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(3.0, 4.0));
    leftAccScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(3.0, 0.0));
    leftPostScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(3.0, -4.0));

    leftScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(-9.0, 2.8));
    leftPieImage = ::QuestUI::BeatSaberUI::CreateImage(modal->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(-9, 3), UnityEngine::Vector2(14, 14));
    leftPieImage->set_color(UnityEngine::Color(0.8f, 0.2f, 0.2f, 0.1f));
    leftPieImage->set_material(UnityEngine::Object::Instantiate(BundleLoader::handAccIndicatorMaterial));

    rightScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(9.0, 2.8));
    rightPieImage = ::QuestUI::BeatSaberUI::CreateImage(modal->get_transform(), Sprites::get_TransparentPixel(), UnityEngine::Vector2(9, 3), UnityEngine::Vector2(14, 14));
    rightPieImage->set_color(UnityEngine::Color(0.2f, 0.2f, 0.8f, 0.1f));
    rightPieImage->set_material(UnityEngine::Object::Instantiate(BundleLoader::handAccIndicatorMaterial));

    rightPreScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(-3.0, 6.0));
    rightAccScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(-3.0, 2.0));
    rightPostScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(-3.0, -2.0));

    FillPropertyId = UnityEngine::Shader::PropertyToID("_FillValue");
}

static float CalculateFillValue(float score) {
    float a = (score - 65) / 50.0f;
    float ratio = a > 1 ? 1.0f : (a < 0 ? 0 : a);
    return pow(ratio, 0.6f);
}

void BeatLeader::ScoreStatsOverview::setScore(optional<ScoreStats> score) {
    if (score.has_value()) {
        leftPreScore->SetText(to_string_wprecision(score->accuracyTracker.leftAverageCut[0], 2));
        leftAccScore->SetText(to_string_wprecision(score->accuracyTracker.leftAverageCut[1], 2));
        leftPostScore->SetText(to_string_wprecision(score->accuracyTracker.leftAverageCut[2], 2));

        leftScore->SetText(to_string_wprecision(score->accuracyTracker.accLeft, 2));
        leftScore->set_alignment(TMPro::TextAlignmentOptions::Center);
        leftPieImage->get_material()->SetFloat(FillPropertyId, CalculateFillValue(score->accuracyTracker.accLeft));
        
        rightScore->SetText(to_string_wprecision(score->accuracyTracker.accRight, 2));
        rightScore->set_alignment(TMPro::TextAlignmentOptions::Center);
        rightPieImage->get_material()->SetFloat(FillPropertyId, CalculateFillValue(score->accuracyTracker.accRight));

        rightPreScore->SetText(to_string_wprecision(score->accuracyTracker.rightAverageCut[0], 2));
        rightPreScore->set_alignment(TMPro::TextAlignmentOptions::Right);
        rightAccScore->SetText(to_string_wprecision(score->accuracyTracker.rightAverageCut[1], 2));
        rightAccScore->set_alignment(TMPro::TextAlignmentOptions::Right);
        rightPostScore->SetText(to_string_wprecision(score->accuracyTracker.rightAverageCut[2], 2));
        rightPostScore->set_alignment(TMPro::TextAlignmentOptions::Right);
    } else {
        setSelected(false);
    }
}

void BeatLeader::ScoreStatsOverview::setSelected(bool selected) {
    leftPreScore->get_gameObject()->SetActive(selected);
    leftAccScore->get_gameObject()->SetActive(selected);
    leftPostScore->get_gameObject()->SetActive(selected);
    leftScore->get_gameObject()->SetActive(selected);
    leftPieImage->get_gameObject()->SetActive(selected);
    rightScore->get_gameObject()->SetActive(selected);
    rightPieImage->get_gameObject()->SetActive(selected);
    rightPreScore->get_gameObject()->SetActive(selected);
    rightAccScore->get_gameObject()->SetActive(selected);
    rightPostScore->get_gameObject()->SetActive(selected);
}