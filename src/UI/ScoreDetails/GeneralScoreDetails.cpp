#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ScoreDetails/GeneralScoreDetails.hpp"
#include "include/UI/QuestUI.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "bsml/shared/BSML/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

BeatLeader::GeneralScoreDetails::GeneralScoreDetails(HMUI::ModalView *modal) noexcept {

    datePlayed = CreateText(modal->get_transform(), "", UnityEngine::Vector2(0.0, 11.0));

    modifiedScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(5.0, 2.0));
    accuracy = CreateText(modal->get_transform(), "", UnityEngine::Vector2(26.0, 2.0));
    scorePp = CreateText(modal->get_transform(), "", UnityEngine::Vector2(46.0, 2.0));

    scoreDetails = CreateText(modal->get_transform(), "", UnityEngine::Vector2(5, -12));   
}

string GetStringWithLabel(const string& value, const string& label) {
    string result = "<color=#888888><size=70%>" + label + "\n</size></color>" + value;
    return result;
}

class MyNumPunct : public std::numpunct<char>
{
protected:
    [[nodiscard]] virtual char do_thousands_sep() const { return ' '; }
    [[nodiscard]] virtual std::string do_grouping() const { return "\03"; }
};

string FormatInt(int score) {
    std::stringstream strm;
    strm.imbue( std::locale( std::locale::classic(), new MyNumPunct ) );
    strm << score;
    return strm.str();
}

string FormatScore(Score const& score) {
    string result = FormatInt(score.modifiedScore);

    if (score.scoreImprovement.score != 0) {
        result += "\n<color=#008800><size=55%>" + to_string_wprecision((score.scoreImprovement.score > 0) ? "+" : "") + FormatInt(score.scoreImprovement.score) + "</size></color>";
    }
    return result;
}

string GetTimeSetString(Score const& score) {
    stringstream result;
    result << "<color=#FFFFFF>" << FormatUtils::GetRelativeTimeString(score.timeset);
    result << "<color=#888888><size=70%>   on   </size>";
    result << "<color=#FFFFFF>" << FormatUtils::GetHeadsetNameById(score.hmd);
    return result.str();
}

string GetDetailsString(const Score& score) {
    stringstream result;

    result << "<color=#888888>Pauses: <color=#FFFFFF>" << to_string(score.pauses) << "    ";
    if (score.modifiers.length() == 0) {
        result << "<color=#888888>No Modifiers\n";
    } else {
        result << "<color=#888888>Modifiers: <color=#FFFFFF>" << score.modifiers << "\n";
    }

    result << "<size=70%>";
    if (score.fullCombo) result << "<color=#88FF88>Full Combo</color>    ";
    if (score.missedNotes > 0) result << "<color=#888888>Misses: <color=#FF8888>" << to_string(score.missedNotes) + "</color>    ";
    if (score.badCuts > 0) result << "<color=#888888>Bad cuts: <color=#FF8888>" << to_string(score.badCuts) + "</color>    ";
    if (score.bombCuts > 0) result << "<color=#888888>Bomb cuts: <color=#FF8888>" << to_string(score.bombCuts) + "</color>    ";
    if (score.wallsHit > 0) result << "<color=#888888>Walls hit: <color=#FF8888>" << to_string(score.wallsHit) + "</color>    ";
    result << "</size>";

    return result.str();
}

inline string FormatAcc(const Score& score) {
    string result = FormatUtils::formatAcc(score.accuracy);

    if (score.scoreImprovement.score != 0) {
       result += "\n<color=#008800><size=55%>" + to_string_wprecision((score.scoreImprovement.accuracy > 0) ? "+" : "") + to_string_wprecision(score.scoreImprovement.accuracy * 100, 2) + "%</size></color>";
    }

    return result;
}

inline string FormatPP(const Score& score) {
    string result = FormatUtils::FormatPP(score.pp);
    if (score.scoreImprovement.score > 0) {
       result += "\n<color=#008800><size=55%>+" + to_string_wprecision(score.scoreImprovement.pp, 2) + "</size></color>";
    }
    return result;
}

void BeatLeader::GeneralScoreDetails::setScore(const Score& score) const {
    datePlayed->SetText(GetTimeSetString(score), true);
    datePlayed->set_alignment(TMPro::TextAlignmentOptions::Center);

    modifiedScore->SetText(GetStringWithLabel(FormatScore(score), "score"), true);
    accuracy->SetText(GetStringWithLabel(FormatAcc(score), "accuracy"), true);
    scorePp->SetText(GetStringWithLabel(FormatPP(score), "pp"), true);

    scoreDetails->SetText(GetDetailsString(score), true);
}

void BeatLeader::GeneralScoreDetails::setSelected(bool selected) const {
    datePlayed->get_gameObject()->SetActive(selected);
    modifiedScore->get_gameObject()->SetActive(selected);
    accuracy->get_gameObject()->SetActive(selected);
    scorePp->get_gameObject()->SetActive(selected);
    scoreDetails->get_gameObject()->SetActive(selected);
}