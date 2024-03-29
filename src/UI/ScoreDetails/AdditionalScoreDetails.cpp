#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/UI/ScoreDetails/AdditionalScoreDetails.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Component.hpp"

#include "main.hpp"

#include <sstream>

using namespace bsml::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

BeatLeader::AdditionalScoreDetails::AdditionalScoreDetails(HMUI::ModalView *modal) noexcept {

    details = CreateText(modal->get_transform(), "", UnityEngine::Vector2(0.0, 2.0));

    positionsTitle = CreateText(modal->get_transform(), "<u>Average HMD position", UnityEngine::Vector2(0, -10));
    positions = CreateText(modal->get_transform(), "", UnityEngine::Vector2(0, -16));   
}

string FormatDetails(string platform, ScoreStats scoreStats) {
    stringstream result;
    auto parts = split(platform, ",");
    if (parts.size() < 3) {
        result << "<color=#888888>Platform: <color=#FFFFFF>Unknown\n";
    } else {
        string platformName = FormatUtils::GetFullPlatformName(parts[0]);
        result << "<color=#888888>Platform: <color=#FFFFFF>" << platformName << "\n";
        result << "<color=#888888>Game: <color=#FFFFFF>" << parts[1] << "     ";
        result << "<color=#888888>Mod: <color=#FFFFFF>" << parts[2] << "     \n";
    }

    result << "<color=#888888>JD: <color=#FFFFFF>" << to_string_wprecision(scoreStats.winTracker.jumpDistance, 2) << "     ";
    result << "<color=#888888>Height: <color=#FFFFFF>" << to_string_wprecision(scoreStats.winTracker.averageHeight, 2) << "<size=70%>m</size>     \n";

    return result.str();
}

string FormatPositions(string platform, ScoreStats scoreStats) {
    stringstream result;

    auto averageHeadPosition = scoreStats.winTracker.averageHeadPosition;
    
    result << "<color=#FF8888><size=80%>X</size>  " << to_string_wprecision(averageHeadPosition.x, 2) << "<size=70%>m</size>    ";
    result << "<color=#88FF88><size=80%>Y</size>  " << to_string_wprecision(averageHeadPosition.y, 2) << "<size=70%>m</size>    ";
    result << "<color=#8888FF><size=80%>Z</size>  " << to_string_wprecision(averageHeadPosition.z, 2) << "<size=70%>m</size>    ";

    return result.str();
}

void BeatLeader::AdditionalScoreDetails::setScore(string platform, ScoreStats score) const {
    details->SetText(FormatDetails(platform, score));
    details->set_alignment(TMPro::TextAlignmentOptions::Center);

    positions->SetText(FormatPositions(platform, score));
    positions->set_alignment(TMPro::TextAlignmentOptions::Center);

    positionsTitle->set_alignment(TMPro::TextAlignmentOptions::Center);
}

void BeatLeader::AdditionalScoreDetails::setSelected(bool selected) const {
    details->get_gameObject()->SetActive(selected);
    positionsTitle->get_gameObject()->SetActive(selected);
    positions->get_gameObject()->SetActive(selected);
}