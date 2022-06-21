#include "include/UI/ScoreDetails/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/UI/ScoreDetails/GeneralScoreDetails.hpp"

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

BeatLeader::GeneralScoreDetails::GeneralScoreDetails(HMUI::ModalView *modal) noexcept {

    datePlayed = CreateText(modal->get_transform(), "", UnityEngine::Vector2(0.0, 11.0));

    modifiedScore = CreateText(modal->get_transform(), "", UnityEngine::Vector2(5.0, 2.0));
    accuracy = CreateText(modal->get_transform(), "", UnityEngine::Vector2(26.0, 2.0));
    scorePp = CreateText(modal->get_transform(), "", UnityEngine::Vector2(46.0, 2.0));

    scoreDetails = CreateText(modal->get_transform(), "", UnityEngine::Vector2(5, -8));

    sponsorMessage = CreateText(modal->get_transform(), "", UnityEngine::Vector2(5, -20));
}

string GetStringWithLabel(string value, string label) {
    string result = "";
    result += "<color=#888888><size=70%>" + label + "\n</size></color>" + value;
    return result;
}

class MyNumPunct : public std::numpunct<char>
{
protected:
    virtual char do_thousands_sep() const { return ' '; }
    virtual std::string do_grouping() const { return "\03"; }
};

string FormatScore(int score) {
    std::stringstream strm;
    strm.imbue( std::locale( std::locale::classic(), new MyNumPunct ) );

    strm << score << std::endl;
    return strm.str();
}

string GetTimeSetString(Score const& score) {
    stringstream result;
    result << "<color=#FFFFFF>" << FormatUtils::GetRelativeTimeString(score.timeset);
    result << "<color=#888888><size=70%>   on   </size>";
    result << "<color=#FFFFFF>" << FormatUtils::GetHeadsetNameById(score.hmd);
    return result.str();
}

string GetDetailsString(Score score) {
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

void BeatLeader::GeneralScoreDetails::setScore(const Score& score) {
    datePlayed->SetText(il2cpp_utils::createcsstr(GetTimeSetString(score)));
    datePlayed->set_alignment(TMPro::TextAlignmentOptions::Center);

    modifiedScore->SetText(il2cpp_utils::createcsstr(GetStringWithLabel(FormatScore(score.modifiedScore), "score")));
    accuracy->SetText(il2cpp_utils::createcsstr(GetStringWithLabel(FormatUtils::formatAcc(score.accuracy), "accuracy")));
    scorePp->SetText(il2cpp_utils::createcsstr(GetStringWithLabel(FormatUtils::FormatPP(score.pp), "pp")));

    scoreDetails->SetText(il2cpp_utils::createcsstr(GetDetailsString(score)));

    sponsorMessage->SetText(il2cpp_utils::createcsstr(score.player.sponsorMessage));
    sponsorMessage->set_alignment(TMPro::TextAlignmentOptions::Center);
}

void BeatLeader::GeneralScoreDetails::setSelected(bool selected) {
    datePlayed->get_gameObject()->SetActive(selected);
    modifiedScore->get_gameObject()->SetActive(selected);
    accuracy->get_gameObject()->SetActive(selected);
    scorePp->get_gameObject()->SetActive(selected);
    scoreDetails->get_gameObject()->SetActive(selected);
    sponsorMessage->get_gameObject()->SetActive(selected);
}