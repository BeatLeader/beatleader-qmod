#include "include/UI/ScoreDetailsUI.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/UI/EmojiSupport.hpp"

#include "UnityEngine/Resources.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/UI/ColorBlock.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

#include "main.hpp"

#include <sstream>

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;

void BeatLeader::initModalPopup(BeatLeader::ModalPopup** modalUIPointer, Transform* parent){
    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::ModalPopup*) malloc(sizeof(BeatLeader::ModalPopup));
    UnityEngine::Sprite* roundRect = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Sprite*>().FirstOrDefault([](UnityEngine::Sprite* x) { return x->get_name() == "RoundRect10"; });
    modalUI->modal = CreateModal(parent, UnityEngine::Vector2(60, 30), [](HMUI::ModalView *modal) {}, true);

    auto playerAvatarImage = ::QuestUI::BeatSaberUI::CreateImage(modalUI->modal->get_transform(), Sprites::get_BeatLeaderIcon(), UnityEngine::Vector2(0, 30), UnityEngine::Vector2(24, 24));
    modalUI->playerAvatar = playerAvatarImage->get_gameObject()->AddComponent<BeatLeader::PlayerAvatar*>();
    modalUI->playerAvatar->Init(playerAvatarImage);

    modalUI->rank = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(6.0, 16.0));
    
    modalUI->name = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(0.0, 18.0));
    modalUI->name->get_gameObject()->AddComponent<::QuestUI::Backgroundable*>()->ApplyBackground("round-rect-panel");

    EmojiSupport::AddSupport(modalUI->name);

    modalUI->pp = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(45.0, 16.0));

    modalUI->datePlayed = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(0.0, 11.0));

    modalUI->modifiedScore = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(5.0, 2.0));
    modalUI->accuracy = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(26.0, 2.0));
    modalUI->scorePp = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(46.0, 2.0));

    modalUI->scoreDetails = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(5, -8));

    modalUI->sponsorMessage = CreateText(modalUI->modal->get_transform(), "", UnityEngine::Vector2(5, -20));

    modalUI->modal->set_name("ScoreDetailsModal");
    *modalUIPointer = modalUI;
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
    if (score.badCuts > 0) result << "<color=#888888>Bad cuts: <color=#FF8888>" << to_string(score.badCuts) + "</color>    \n";
    if (score.bombCuts > 0) result << "<color=#888888>Bomb cuts: <color=#FF8888>" << to_string(score.bombCuts) + "</color>    ";
    if (score.wallsHit > 0) result << "<color=#888888>Walls hit: <color=#FF8888>" << to_string(score.wallsHit) + "</color>    ";
    result << "</size>";

    return result.str();
}

void BeatLeader::ModalPopup::setScore(const Score& score) {
    playerAvatar->SetPlayer(score.player.avatar, score.player.role);

    name->SetText(FormatUtils::FormatNameWithClans(score.player));
    name->set_alignment(TMPro::TextAlignmentOptions::Center);
    rank->SetText(FormatUtils::FormatRank(score.player.rank, true));
    pp->SetText(FormatUtils::FormatPP(score.player.pp));

    datePlayed->SetText(GetTimeSetString(score));
    datePlayed->set_alignment(TMPro::TextAlignmentOptions::Center);

    modifiedScore->SetText(GetStringWithLabel(FormatScore(score.modifiedScore), "score"));
    accuracy->SetText(GetStringWithLabel(FormatUtils::formatAcc(score.accuracy), "accuracy"));
    scorePp->SetText(GetStringWithLabel(FormatUtils::FormatPP(score.pp), "pp"));

    scoreDetails->SetText(GetDetailsString(score));

    sponsorMessage->SetText(score.player.sponsorMessage);
    sponsorMessage->set_alignment(TMPro::TextAlignmentOptions::Center);
}