#include "include/UI/Results/DanceResultsUI.hpp"
#include "include/Assets/DanceEventAssets.hpp"
#include "include/Assets/Sprites.hpp"
#include "include/Core/Dance/DanceDread.hpp"
#include "include/Core/Dance/DanceEvent.hpp"
#include "include/Core/Dance/DanceModeTracker.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/main.hpp"

#include "shared/Models/DanceEventStatus.hpp"

#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "BGLib/Polyglot/LocalizedTextMeshProUGUI.hpp"
#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/FontStyles.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Behaviour.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include <cmath>
#include <fmt/format.h>

DEFINE_TYPE(BeatLeader, DanceEventResultsPanel);

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace DanceResultsUI {
    static Vector2 Origin(bool cleared) {
        return cleared ? Vector2(2, -35) : Vector2(2, -24);
    }

    static Vector2 Offset(Vector2 origin, float x, float y) {
        return Vector2(origin.x + x, origin.y + y);
    }

    static SafePtrUnity<TMPro::TextMeshProUGUI> verdictText;
    static SafePtrUnity<TMPro::TextMeshProUGUI> danceText;
    static SafePtrUnity<HMUI::ImageView> tombstoneImage;
    static SafePtrUnity<TMPro::TextMeshProUGUI> tombstoneText;
    static SafePtrUnity<BeatLeader::DanceEventResultsPanel> eventPanel;

    #pragma region Construct

    static TMPro::TextMeshProUGUI* CreateText(Transform* parent, float fontSize, Vector2 size) {
        auto text = BSML::Lite::CreateText(parent, "", TMPro::FontStyles::Normal, fontSize, {0, 0}, size);
        text->set_alignment(TMPro::TextAlignmentOptions::Center);
        text->set_richText(true);
        text->set_enableWordWrapping(true);
        text->get_gameObject()->SetActive(false);
        return text;
    }

    static void CreatePanel(Transform* parent) {
        auto go = GameObject::New_ctor("BeatLeaderDanceEventResults");
        auto rect = go->AddComponent<RectTransform*>();
        rect->SetParent(parent, false);
        rect->set_anchorMin({0.5f, 0.5f});
        rect->set_anchorMax({0.5f, 0.5f});
        rect->set_pivot({0.5f, 0.5f});
        rect->set_sizeDelta({76, 12});
        rect->set_anchoredPosition({0, 52});
        auto host = go->AddComponent<BeatLeader::DanceEventResultsPanel*>();
        BSML::parse_and_construct(R"(
            <horizontal bg="round-rect-panel" bg-color="#000000cc" pad="1" spacing="1" child-expand-width="false" child-align="MiddleLeft">
                <image id="_image" pref-width="10" pref-height="10" preserve-aspect="true"/>
                <vertical pref-width="56" spacing="0" child-expand-height="false">
                    <text id="_title" text="" font-size="4" italics="true" overflow-mode="Ellipsis" word-wrapping="false" align="Left" rich-text="true"/>
                    <text id="_result" text="" font-size="3.5" italics="true" overflow-mode="Ellipsis" word-wrapping="false" align="Left" rich-text="true"/>
                </vertical>
                <loading-indicator id="_loading" pref-width="5" pref-height="5" preserve-aspect="true"/>
            </horizontal>
        )", rect, host);
        host->_image->set_material(BSML::Helpers::GetUINoGlowMat());
        eventPanel = host;
        go->SetActive(false);
    }

    static void Construct(ResultsViewController* resultsView) {
        auto transform = resultsView->get_transform();
        verdictText = CreateText(transform, 3, {140, 5});
        danceText = CreateText(transform, 3.5f, {120, 5});

        tombstoneImage = BSML::Lite::CreateImage(transform, DanceEventAssets::GetSprite("rip-tombstone.png"), {0, 0}, {26, 29});
        tombstoneImage->set_preserveAspect(true);
        tombstoneImage->get_gameObject()->SetActive(false);
        tombstoneText = CreateText(transform, 3.5f, {22, 8});
        tombstoneText->set_color(Color(0.1f, 0.1f, 0.1f, 1));

        CreatePanel(transform);
    }

    static void Place(Component* component, Vector2 position) {
        component->get_transform().cast<RectTransform>()->set_anchoredPosition(position);
    }

    #pragma endregion

    #pragma region Dance score

    static std::string Thousands(int64_t value) {
        auto digits = std::to_string(value < 0 ? -value : value);
        std::string result;
        for (size_t i = 0; i < digits.size(); i++) {
            if (i > 0 && (digits.size() - i) % 3 == 0) result += ',';
            result += digits[i];
        }
        return value < 0 ? "-" + result : result;
    }

    static void SetupDanceScore(std::optional<Dance::DanceResult> const& result) {
        danceText->get_gameObject()->SetActive(result.has_value());
        if (!result) return;
        if (!result->Valid) {
            danceText->set_text("Dance score: not enough play to judge");
            return;
        }
        auto score = Thousands((int64_t)std::round(result->Score * 1'000'000));
        danceText->set_text(fmt::format(
            "Dance score <color=#FFD700>{}</color>  <color=#AAAAAA>~{:.0f}% of players would call this dancing</color>",
            score, result->VoteShare * 100));
    }

    #pragma endregion

    #pragma region Verdict

    static std::string FormatVerdict(DanceDread::Summary const& dread) {
        if (dread.Died) {
            return dread.Escapes > 0
                ? "R.I.P. You fought back, but the ghosts won"
                : "R.I.P. The ghosts took you without a fight";
        }
        if (dread.HauntedSeconds < 1) return "Not a single ghost dared to come near";
        float hauntedShare = dread.PlayedSeconds > 0 ? dread.HauntedSeconds / dread.PlayedSeconds : 0;
        return dread.Escapes > 0 && hauntedShare < 0.25f
            ? "The ghosts tried, you danced them away"
            : "You made it out... barely";
    }

    static void SetupVerdict(std::optional<DanceDread::Summary> const& dread) {
        verdictText->get_gameObject()->SetActive(dread.has_value());
        if (!dread) return;
        auto haunted = dread->HauntedSeconds < 1
            ? std::string("never haunted")
            : fmt::format("haunted for {:.0f} s", dread->HauntedSeconds);
        verdictText->set_text(fmt::format("<color=#C9A0FF><i>{}</i></color>   <color=#AAAAAA>{}</color>", FormatVerdict(*dread), haunted));
    }

    static void SetupTombstone(std::optional<DanceDread::Summary> const& dread) {
        bool active = dread && dread->Died && tombstoneImage->get_sprite();
        tombstoneImage->get_gameObject()->SetActive(active);
        tombstoneText->get_gameObject()->SetActive(active);
        if (!active) return;
        int death = (int)std::max(0.0f, dread->DeathTime);
        tombstoneText->set_text(fmt::format("died {}:{:02}\n<size=80%>still for {:.0f} s</size>", death / 60, death % 60, dread->LongestStillSeconds));
    }

    #pragma endregion

    #pragma region Banner

    struct OverriddenText {
        SafePtrUnity<TMPro::TMP_Text> text;
        std::string original;
        SafePtrUnity<Behaviour> localizer;
    };

    static std::vector<OverriddenText> overriddenBannerTexts;

    static void RestoreBanner() {
        for (auto& overridden : overriddenBannerTexts) {
            if (!overridden.text) continue;
            overridden.text->set_text(overridden.original);
            if (overridden.localizer) overridden.localizer->set_enabled(true);
        }
        overriddenBannerTexts.clear();
    }

    static void OverrideBanner(GameObject* banner, std::string const& text) {
        auto labels = banner->GetComponentsInChildren<TMPro::TMP_Text*>(true);
        for (auto label : labels) {
            Behaviour* localizer = label->GetComponent<BGLib::Polyglot::LocalizedTextMeshProUGUI*>();
            if (localizer) localizer->set_enabled(false);
            overriddenBannerTexts.push_back({label, label->get_text() ? (std::string)label->get_text() : "", localizer});
            label->set_text(text);
        }
    }

    static void SetupBanner(ResultsViewController* resultsView, std::optional<DanceDread::Summary> const& dread) {
        RestoreBanner();
        if (!dread) return;
        OverrideBanner(resultsView->_failedBannerGo, "DIED");
        OverrideBanner(resultsView->_clearedBannerGo, "SURVIVED");
    }

    #pragma endregion

    #pragma region Event points

    static int setupGeneration = 0;
    static std::optional<PlatformEvent> pointsEvent;
    static int pointsPlay = -1;
    static bool waitingForUpload = false;

    static void SetTitle(std::string const& standing) {
        auto const& name = pointsEvent->name;
        eventPanel->_title->set_text(standing.empty() ? name : fmt::format("{}  <color=#AAAAAA>{}</color>", name, standing));
    }

    static void SetResult(std::string const& text, std::string const& standing = "") {
        if (!eventPanel) return;
        SetTitle(standing);
        eventPanel->_result->set_text(text);
        eventPanel->_loading->SetActive(false);
    }

    static std::string FormatEventResult(DanceEventStatus const& status) {
        auto const& score = *status.score;
        if (!score.eligible) return "Can't count, too little was judged";
        int best = status.countedMaps;
        auto counted = score.counted ? fmt::format("in your best {}", best) : fmt::format("not in your best {}", best);
        return fmt::format("<color=#FF9A1F>{:.1f} points</color>, {}", score.points, counted);
    }

    static void HandleUploadDone(PlayUpload const& upload) {
        waitingForUpload = false;
        auto const& response = upload.result;
        if (response && response->status == ScoreUploadStatus::Uploaded && response->score) {
            int generation = setupGeneration;
            DanceEventStatus::Load(pointsEvent->id, response->score->id, [generation](std::optional<DanceEventStatus> status) {
                if (generation != setupGeneration) return;
                if (status && status->score) {
                    auto standing = status->player ? fmt::format("#{} with {:.1f}", status->player->rank, status->player->points) : "";
                    SetResult(FormatEventResult(*status), standing);
                } else {
                    SetResult("The event standings didn't load");
                }
            });
            return;
        }
        if (response && response->status == ScoreUploadStatus::NonPB) {
            SetResult("Not your best on this map");
        } else if (response && response->status == ScoreUploadStatus::Attempt) {
            SetResult("Only cleared runs count for the event");
        } else {
            SetResult("Couldn't send this run");
        }
    }

    static void HandlePlayUploadChanged() {
        if (!waitingForUpload || !pointsEvent) return;
        auto const& upload = ReplayManager::LastPlayUpload();
        if (upload.play != pointsPlay || !upload.done) return;
        HandleUploadDone(upload);
    }

    static void WaitForEventPoints(std::optional<PlatformEvent> const& evt) {
        pointsEvent = evt;
        waitingForUpload = false;
        eventPanel->get_gameObject()->SetActive(evt.has_value());
        if (!evt) return;

        SetTitle("");
        eventPanel->_result->set_text("");
        eventPanel->_loading->SetActive(true);
        eventPanel->_image->set_sprite(DanceEventAssets::GetSprite("pumpkin.png"));
        int generation = setupGeneration;
        Sprites::get_Icon(evt->image, [generation](Sprite* sprite) {
            if (generation != setupGeneration || !sprite || !eventPanel) return;
            eventPanel->_image->set_sprite(sprite);
        });

        auto const& upload = ReplayManager::LastPlayUpload();
        pointsPlay = upload.play;
        waitingForUpload = true;
        if (upload.done) HandleUploadDone(upload);
    }

    #pragma endregion

    void Setup(ResultsViewController* resultsView, bool lastGameWasReplay) {
        setupGeneration++;
        if (!verdictText || !eventPanel) Construct(resultsView);

        auto results = resultsView->_levelCompletionResults;
        bool cleared = results && results->levelEndStateType == LevelCompletionResults::LevelEndStateType::Cleared;
        auto origin = Origin(cleared);
        Place(verdictText.ptr(), Offset(origin, 0, 11.8f));
        Place(danceText.ptr(), Offset(origin, 0, -5.3f));
        Place(tombstoneImage.ptr(), Offset(origin, -71, -3.5f));
        Place(tombstoneText.ptr(), Offset(origin, -71, -5.1f));

        std::optional<Dance::DanceResult> danceResult = lastGameWasReplay ? std::nullopt : DanceModeTracker::LastResult();
        std::optional<DanceDread::Summary> dread = danceResult ? DanceDread::LastSummary() : std::nullopt;

        SetupDanceScore(danceResult);
        SetupVerdict(dread);
        SetupTombstone(dread);
        SetupBanner(resultsView, dread);
        bool submitted = dread && ReplayManager::LastPlayUpload().started;
        WaitForEventPoints(submitted ? DanceEvent::Current() : std::nullopt);
    }

    void setup() {
        ReplayManager::AddPlayUploadListener(HandlePlayUploadChanged);
    }
}
