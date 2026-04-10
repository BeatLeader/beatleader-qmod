#include "include/UI/CaptorClanUI.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/UI/Components/SimpleClickHandler.hpp"
#include "include/UI/Components/SmoothHoverController.hpp"
#include "include/Utils/FormatUtils.hpp"

#include "GlobalNamespace/ColorExtensions.hpp"

#include "UnityEngine/Vector3.hpp"

#include "bsml/shared/BSML-Lite.hpp"

#include <algorithm>
#include <exception>
#include <string>
#include <string_view>

DEFINE_TYPE(BeatLeader, CaptorClanTagComponent);
DEFINE_TYPE(BeatLeader, CaptorClanComponent);

namespace {
    constexpr float DefaultScale = 1.05f;
    constexpr float HoverScale = 1.1f;
    constexpr float ClanTagScale = 1.1f;
    constexpr float ClanTagWidthPerCharacter = 1.0f;
    constexpr float ClanTagMinWidth = 3.0f;
    constexpr float ClanTagMaxWidth = 5.5f;
    constexpr float HeaderBackgroundAlpha = 221.0f / 255.0f;

    void ApplyHeaderBackgroundColor(HMUI::ImageView* background) {
        if (!background) {
            return;
        }

        auto color = UnityEngine::Color(1.0f, 1.0f, 1.0f, HeaderBackgroundAlpha);
        background->set_color(color);
        background->set_color0(color);
        background->set_color1(color);
    }

    UnityEngine::Color HexToRgb(std::string_view hex) {
        if (hex.size() != 7 || hex.front() != '#') {
            return UnityEngine::Color::get_black();
        }

        try {
            auto parseChannel = [](std::string_view value) {
                return static_cast<float>(std::stoi(std::string(value), nullptr, 16)) / 255.0f;
            };

            return UnityEngine::Color(
                parseChannel(hex.substr(1, 2)),
                parseChannel(hex.substr(3, 2)),
                parseChannel(hex.substr(5, 2)),
                1.0f
            );
        } catch (std::exception const&) {
            return UnityEngine::Color::get_black();
        }
    }

    void NotifyShowClanRankingChanged() {
        if (CaptorClanUI::showClanRankingCallback) {
            CaptorClanUI::showClanRankingCallback();
        }
    }
}

namespace BeatLeader {

    void CaptorClanComponent::Awake() {
        _captorClanTag = CaptorClanTag::Instantiate<CaptorClanTag>(get_transform())->LocalComponent();
    }

    void CaptorClanTag::OnInitialize() {
        LocalComponent()->_backgroundImage->set_material(BundleLoader::bundle->clanTagBackgroundMaterial);
        LocalComponent()->_textComponent->set_enableAutoSizing(true);
        LocalComponent()->_textComponent->set_fontSizeMin(0.1f);
        LocalComponent()->_textComponent->set_fontSizeMax(2.0f);
        LocalComponent()->_textComponent->set_alignment(TMPro::TextAlignmentOptions::Midline);
        Clear();
    }

    StringW CaptorClanTag::GetContent() {
        return StringW(R"(
            <vertical id="container" pref-width="3.0">
                <horizontal id="_backgroundImage" pref-height="2.8" bg="round-rect-panel">
                    <text id="_textComponent" word-wrapping="false" align="Midline"/>
                </horizontal>
            </vertical>
        )");
    }

    void CaptorClanTag::Clear() {
        LocalComponent()->container->get_gameObject()->SetActive(false);
    }

    void CaptorClanTag::SetValue(Clan const& value) {
        LocalComponent()->_textComponent->set_text(FormatUtils::FormatClanTag(value.tag));
        LocalComponent()->container->get_gameObject()->SetActive(true);
        SetColor(value.color);
        SetPreferredWidth(value.tag);
    }

    float CaptorClanTag::CalculatePreferredWidth() const {
        return _preferredWidth;
    }

    void CaptorClanTag::SetPreferredWidth(std::string_view tag) {
        auto unclamped = std::min(
            LocalComponent()->_textComponent->get_preferredWidth(),
            ClanTagWidthPerCharacter * static_cast<float>(tag.size())
        );
        _preferredWidth = std::clamp(unclamped, ClanTagMinWidth, ClanTagMaxWidth);

        auto* containerLayoutElement = LocalComponent()->container->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>();
        if (containerLayoutElement) {
            containerLayoutElement->set_preferredWidth(_preferredWidth);
        }
    }

    void CaptorClanTag::SetColor(std::string_view value) {
        auto color = HexToRgb(value);
        auto useDarkFont = (color.r * 0.299f + color.g * 0.687f + color.b * 0.114f) > 0.73f;
        LocalComponent()->_textComponent->set_color(useDarkFont ? UnityEngine::Color::get_black() : UnityEngine::Color::get_white());
        _color = color;
        UpdateColor();
    }

    void CaptorClanTag::UpdateColor() {
        LocalComponent()->_backgroundImage->set_color(GlobalNamespace::ColorExtensions::ColorWithAlpha(_color, _alpha));
    }

    void CaptorClanView::OnInitialize() {
        LocalComponent()->_background->set_raycastTarget(true);
        ApplyHeaderBackgroundColor(LocalComponent()->_background);
        LocalComponent()->_captorClanText->set_fontSize(4.0f);
        LocalComponent()->_captorClanText->set_alignment(TMPro::TextAlignmentOptions::Midline);

        LocalComponent()->_captorClanTag->container->set_localScale(UnityEngine::Vector3(ClanTagScale, ClanTagScale, ClanTagScale));

        auto root = LocalComponent()->GetRootTransform();
        auto rootObject = root->get_gameObject();
        SimpleClickHandler::Custom(rootObject, [this](bool) {
            CaptorClanUI::setShowClanRanking(!CaptorClanUI::showClanRanking);
            NotifyShowClanRankingChanged();
        });
        SmoothHoverController::Scale(rootObject, DefaultScale, HoverScale);
        LocalComponent()->_hoverHint = BSML::Lite::AddHoverHint(LocalComponent()->_background, "");
        UpdateVisibility();
    }

    StringW CaptorClanView::GetContent() {
        return StringW(R"(
            <horizontal id="_background" pad-left="1" pad-right="1" spacing="2" bg="round-rect-panel">
                <horizontal pref-height="6">
                    <text id="_captorClanText" font-size="4" align="Midline"/>
                </horizontal>
                <macro.as-host host="_captorClanTag">
                    <macro.reparent transform="_uiComponent"/>
                </macro.as-host>
            </horizontal>
        )");
    }

    void CaptorClanView::SetHeaderText(TMPro::TextMeshProUGUI* text) {
        _headerText = text;
        UpdateVisibility();
    }

    void CaptorClanView::SetActive(bool value) {
        _isActive = value;

        if (!value) {
            CaptorClanUI::showClanRanking = false;
        }

        if (value && _lastStatus.has_value()) {
            SetValues(*_lastStatus);
            return;
        }

        UpdateVisibility();
    }

    void CaptorClanView::SetValues(ClanRankingStatus const& status) {
        _lastStatus = status;
        auto* clanTagView = GetClanTagView();

        if (!status.applicable) {
            if (CaptorClanUI::showClanRanking) {
                CaptorClanUI::setShowClanRanking(false);
                NotifyShowClanRankingChanged();
            }

            UpdateVisibility();
            return;
        }

        UpdateVisibility();

        if (!_isActive) {
            return;
        }

        if (!clanTagView) {
            return;
        }

        if (status.clanRankingContested) {
            clanTagView->Clear();
            SetStatusText("⚔ Contested", UnityEngine::Color32(0, 192, 192, 192, 255));
            SetHoverText("Several clans claim equal rights to capture this map! Set a score to break the tie");
            return;
        }

        if (!status.clan.has_value() || status.clan->tag.empty()) {
            clanTagView->Clear();
            SetStatusText("Uncaptured", UnityEngine::Color32(0, 255, 255, 255, 255));
            SetHoverText("Map is not captured! Set a score to capture it for your clan");
            return;
        }

        auto const& clan = *status.clan;
        clanTagView->SetValue(clan);
        SetStatusText("👑", UnityEngine::Color32(0, 255, 215, 0, 255));
        SetHoverText("Map is captured by \r\n<b>" + clan.name + "</b>\r\n They have the highest weighted PP on this leaderboard");
    }

    CaptorClanTag* CaptorClanView::GetClanTagView() {
        return reinterpret_cast<CaptorClanTag*>(LocalComponent()->_captorClanTag->nativeComponent);
    }

    void CaptorClanView::UpdateVisibility() {
        auto visible = _isActive && _lastStatus.has_value() && _lastStatus->applicable;
        LocalComponent()->SetRootActive(visible);

        if (_headerText) {
            _headerText->get_gameObject()->SetActive(!visible);
        }
    }

    void CaptorClanView::SetStatusText(std::string_view text, UnityEngine::Color32 color) {
        LocalComponent()->_captorClanText->set_text(std::string(text));
        LocalComponent()->_captorClanText->set_faceColor(color);
    }

    void CaptorClanView::SetHoverText(std::string const& text) {
        if (LocalComponent()->_hoverHint) {
            LocalComponent()->_hoverHint->set_text(text);
        }
    }
}

namespace CaptorClanUI {
    bool showClanRanking = false;
    std::function<void()> showClanRankingCallback;

    std::optional<ClanRankingStatus> lastStatus;
    bool isActive = true;

    BeatLeader::CaptorClanView* captorClanView = nullptr;

    void Attach(BeatLeader::CaptorClanView* view) {
        captorClanView = view;

        if (!captorClanView) {
            return;
        }

        captorClanView->SetActive(isActive);

        if (lastStatus.has_value()) {
            captorClanView->SetValues(*lastStatus);
        }
    }

    void initCaptorClan(UnityEngine::GameObject* headerPanel, TMPro::TextMeshProUGUI* headerPanelText) {
        auto* view = BeatLeader::CaptorClanView::Instantiate<BeatLeader::CaptorClanView>(headerPanel->get_transform());
        view->LocalComponent()->ManualInit(headerPanel->get_transform());
        view->SetHeaderText(headerPanelText);
        Attach(view);
    }

    void setClan(ClanRankingStatus clanStatus) {
        lastStatus = clanStatus;

        if (captorClanView) {
            captorClanView->SetValues(clanStatus);
        }
    }

    void setActive(bool active) {
        isActive = active;

        if (captorClanView) {
            captorClanView->SetActive(active);
        } else if (!active) {
            showClanRanking = false;
        }
    }

    void setShowClanRanking(bool active) {
        showClanRanking = active;
    }

    void Reset() {
        captorClanView = nullptr;
        lastStatus.reset();
        isActive = true;
    }
}
