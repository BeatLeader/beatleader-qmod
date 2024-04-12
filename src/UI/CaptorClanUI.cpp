#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/LevelCollectionNavigationController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/StandardLevelDetailViewController.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSegmentedControlController.hpp"
#include "GlobalNamespace/LevelParamsPanel.hpp"
#include "GlobalNamespace/ColorExtensions.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "include/Assets/Sprites.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/Models/Song.hpp"
#include "include/Models/Difficulty.hpp"
#include "include/Models/TriangleRating.hpp"
#include "include/UI/CaptorClanUI.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/UI/UIUtils.hpp"
#include "include/UI/EmojiSupport.hpp"
#include "include/Utils/FormatUtils.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "main.hpp"

#include "TMPro/TMP_Text.hpp"

#include "bsml/shared/bsml.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include <numeric>
#include <map>
#include <string>
#include <regex>

using namespace GlobalNamespace;
using namespace std;
using namespace BSML;
using namespace Lite;

namespace CaptorClanUI {
    bool showClanRanking = false;
    function<void()> showClanRankingCallback;

    ClanRankingStatus lastStatus = ClanRankingStatus();
    bool isActive = true;

    TMPro::TextMeshProUGUI* headerText;

    UnityEngine::UI::HorizontalLayoutGroup* mainPanel;
    TMPro::TextMeshProUGUI* clanTag;
    TMPro::TextMeshProUGUI* captorClanStatus;

    BSML::ClickableImage* backgroundImage;
    HMUI::ImageView* clanImage;

    HMUI::HoverHint* clanHint;

    UnityEngine::Color color;
    float alpha = 1.0;
    const float WidthPerCharacter = 2.3f;

    UnityEngine::Color hexToRGB(const std::string& hex) {
        // Assuming hex is in the format "#RRGGBB"
        if (hex.length() != 7 || hex[0] != '#') {
            throw std::invalid_argument("Invalid hex color format");
        }

        int r, g, b;
        std::stringstream ss;
        ss << std::hex << hex.substr(1, 2);
        ss >> r;
        ss.clear();
        ss << std::hex << hex.substr(3, 2);
        ss >> g;
        ss.clear();
        ss << std::hex << hex.substr(5, 2);
        ss >> b;

        return UnityEngine::Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }

    UnityEngine::Color BackgroundColor() {
        return showClanRanking ? hexToRGB("#822cb8") : UnityEngine::Color::get_black();
    }

    UnityEngine::Color BackgroundHoverColor() {
        return hexToRGB("#1e1a69");
    }

    void initCaptorClan(UnityEngine::GameObject* headerPanel, TMPro::TextMeshProUGUI* headerPanelText) {
        headerText = headerPanelText;

        backgroundImage = CreateClickableImage(headerPanel->get_transform(), Sprites::get_TransparentPixel(), []() {
            showClanRanking = !showClanRanking;
            showClanRankingCallback();
        }, {0, 0}, {17, 6});
        backgroundImage->set_material(BundleLoader::bundle->clanTagBackgroundMaterial);

        backgroundImage->set_color(BackgroundColor());
        backgroundImage->onEnter = [](){ 
            backgroundImage->set_color(BackgroundHoverColor());
        };

        backgroundImage->onExit = [](){ 
            backgroundImage->set_color(BackgroundColor());
        };

        mainPanel = Lite::CreateHorizontalLayoutGroup(backgroundImage->get_transform());
        mainPanel->set_padding(RectOffset::New_ctor(1, 1, 0, 0));
        mainPanel->set_spacing(3);
        mainPanel->GetComponentInChildren<UnityEngine::UI::ContentSizeFitter*>()->set_horizontalFit(UnityEngine::UI::ContentSizeFitter::FitMode::PreferredSize);
        mainPanel->GetComponentInChildren<UnityEngine::UI::LayoutElement*>()->set_preferredHeight(5.0f);
        clanHint = AddHoverHint(mainPanel, "");
        
        captorClanStatus = Lite::CreateText(mainPanel->get_transform(), "");
        captorClanStatus->set_fontSize(3.0f);
        captorClanStatus->set_alignment(TMPro::TextAlignmentOptions::Midline);
        EmojiSupport::AddSupport(captorClanStatus);

        clanImage = UIUtils::CreateRoundRectImage(mainPanel->get_transform(), {0, 0}, {8, 2.5});
        clanImage->set_material(BundleLoader::bundle->clanTagBackgroundMaterial);

        clanTag = CreateText(clanImage->get_transform(), "", UnityEngine::Vector2(0.0, 0.0));
        clanTag->set_enableAutoSizing(true);
        clanTag->set_fontSizeMin(0.1);
        clanTag->set_fontSizeMax(3.0);
        clanTag->set_alignment(TMPro::TextAlignmentOptions::Midline);

        setClan(lastStatus);
    }

    void setClan(ClanRankingStatus clanStatus) {
        lastStatus = clanStatus;

        if (mainPanel == NULL || !isActive) return;

        clanImage->get_gameObject()->SetActive(false);

        if (!clanStatus.applicable) {
            if (showClanRanking) {
                showClanRanking = false;
                showClanRankingCallback();
            }
            backgroundImage->get_gameObject()->SetActive(false);
            mainPanel->get_gameObject()->SetActive(false);
            if (headerText != NULL) {
                headerText->get_gameObject()->SetActive(true);
            }
            return;
        } else {
            headerText->get_gameObject()->SetActive(false);
            backgroundImage->get_gameObject()->SetActive(true);
            mainPanel->get_gameObject()->SetActive(true);
        }

        float backgroundWidth = 0;
        float backgroundHeight = 5;

        if (clanStatus.clan != nullopt) {
            clanImage->get_gameObject()->SetActive(true);

            auto clan = clanStatus.clan;
            string clanText = "<alpha=#00>.<alpha=#FF><b><noparse>" + clan->tag + "</noparse></b><alpha=#00>.<alpha=#FF>";
            clanTag->set_text(clanText);
            bool useDarkFont = (color.r * 0.299f + color.g * 0.687f + color.b * 0.114f) > 0.73f;
            clanTag->set_color(useDarkFont ? UnityEngine::Color::get_black() : UnityEngine::Color::get_white());
            
            color = hexToRGB(clan->color);

            clanImage->GetComponentInChildren<UnityEngine::UI::LayoutElement*>()->set_preferredWidth(WidthPerCharacter * clan->tag.length());
            clanImage->set_color(ColorExtensions::ColorWithAlpha(color, alpha));
            
            string text = "👑   ";
            captorClanStatus->set_text(text);
            captorClanStatus->set_faceColor(UnityEngine::Color32(0, 255, 215, 0, 255));

            clanHint->set_text("Map is captured by \r\n" + clan->name + "\r\n They have the highest weighted PP on this leaderboard");
            backgroundWidth = WidthPerCharacter * clan->tag.length() + WidthPerCharacter * text.length() / 2;
            backgroundHeight = 7;
        } else if (clanStatus.clanRankingContested) {
            string text = "⚔   Contested ";
            captorClanStatus->set_text(text);
            captorClanStatus->set_faceColor(UnityEngine::Color32(0, 192, 192, 192, 255));

            clanHint->set_text("Several clans claim equal rights to capture this map! Set a score to break the tie");
            backgroundWidth = WidthPerCharacter * text.length() / 2;
 
        } else {
            string text = "👑   Uncaptured ";
            captorClanStatus->set_text(text);
            captorClanStatus->set_faceColor(UnityEngine::Color32(0, 255, 255, 255, 255));

            clanHint->set_text("Map is not captured! Set a score to capture it for your clan");
            backgroundWidth = WidthPerCharacter * text.length() / 2;
        }

        auto rectTransform = backgroundImage->get_rectTransform();
        rectTransform->set_sizeDelta({backgroundWidth, backgroundHeight});
    }

    void setActive(bool active) {
        isActive = active;
        if (!active) {
            showClanRanking = false;
            if (backgroundImage) {
                backgroundImage->get_gameObject()->SetActive(false);
                mainPanel->get_gameObject()->SetActive(false);
                if (headerText != NULL) {
                    headerText->get_gameObject()->SetActive(true);
                }
            }
        } else {
            setClan(lastStatus);
        }
    }
}