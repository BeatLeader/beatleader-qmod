#pragma once

#include "shared/Models/Clan.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "HMUI/HoverHint.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Color32.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include <functional>
#include <optional>
#include <string>
#include <string_view>

DECLARE_CLASS_CUSTOM(BeatLeader, CaptorClanTagComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, container);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _backgroundImage);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _textComponent);
};

DECLARE_CLASS_CUSTOM(BeatLeader, CaptorClanComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _background);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _captorClanText);
    DECLARE_INSTANCE_FIELD(BeatLeader::CaptorClanTagComponent*, _captorClanTag);
    DECLARE_INSTANCE_FIELD(HMUI::HoverHint*, _hoverHint);

    DECLARE_INSTANCE_METHOD(void, Awake);
};

namespace BeatLeader {

class CaptorClanTag : public ReeUIComponentV2<CaptorClanTagComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void Clear();
    void SetValue(Clan const& value);
    float CalculatePreferredWidth() const;

private:
    void SetPreferredWidth(std::string_view tag);
    void SetColor(std::string_view value);
    void UpdateColor();

    float _alpha = 1.0f;
    float _preferredWidth = 0.0f;
    UnityEngine::Color _color = UnityEngine::Color::get_black();
};

class CaptorClanView : public ReeUIComponentV2<CaptorClanComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void SetHeaderText(TMPro::TextMeshProUGUI* text);
    void SetActive(bool value);
    void SetValues(ClanRankingStatus const& status);

private:
    CaptorClanTag* GetClanTagView();
    void UpdateVisibility();
    void SetStatusText(std::string_view text, UnityEngine::Color32 color);
    void SetHoverText(std::string const& text);

    TMPro::TextMeshProUGUI* _headerText = nullptr;
    std::optional<ClanRankingStatus> _lastStatus;
    bool _isActive = true;
};

}

namespace CaptorClanUI {
    extern bool showClanRanking;
    extern std::function<void()> showClanRankingCallback;

    void Attach(BeatLeader::CaptorClanView* view);
    void initCaptorClan(UnityEngine::GameObject* header, TMPro::TextMeshProUGUI* headerPanelText);
    void setClan(ClanRankingStatus clanStatus);
    void setActive(bool active);
    void setShowClanRanking(bool active);
    void Reset();
}
