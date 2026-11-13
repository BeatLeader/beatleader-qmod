#include "UI/MainMenu/DanceEvent/DanceEventPanel.hpp"
#include "UI/MainMenu/Modals/EventDetailsDialog.hpp"
#include "UI/Abstract/ReeModalSystem.hpp"
#include "Assets/DanceEventAssets.hpp"

#include "UnityEngine/Color.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

#include "songcore/shared/SongCore.hpp"
#include "custom-types/shared/delegate.hpp"

#include <fmt/format.h>

DEFINE_TYPE(BeatLeader, DanceEventPanelComponent);

namespace BeatLeader {
    static const UnityEngine::Color PumpkinOrange = UnityEngine::Color(1.0f, 0.48f, 0.1f, 1.0f);
    static constexpr const char* LoadingMessage = "Summoning the standings...";
    static constexpr size_t MaxMapNameLength = 28;

    static SafePtrUnity<UnityEngine::RectTransform> shownContent;

    UnityEngine::RectTransform* DanceEventPanel::Shown() {
        if (!shownContent || !shownContent->get_gameObject()->get_activeInHierarchy()) return nullptr;
        return shownContent.ptr();
    }

    void DanceEventPanel::SetShown(UnityEngine::RectTransform* content) {
        shownContent = content;
    }

    DanceEventPanelComponent* DanceEventPanel::Usable() {
        if (!safeComponent || safeComponent->_state != ReeComponentState::HierarchySet) return nullptr;
        return safeComponent.ptr();
    }

    static void AddClick(UnityEngine::UI::Button* button, std::function<void()> action) {
        button->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(action));
    }

    void DanceEventPanel::OnInitialize() {
        auto component = LocalComponent();
        safeComponent = component;

        component->_background->set_raycastTarget(true);
        // A flexible panel would stretch over the news list
        component->_background->GetComponent<UnityEngine::UI::LayoutElement*>()->set_flexibleWidth(0);
        component->_headerBackground->set_color(UnityEngine::Color(PumpkinOrange.r, PumpkinOrange.g, PumpkinOrange.b, 0.8f));
        component->_points->set_color(PumpkinOrange);
        component->_pumpkin->set_sprite(DanceEventAssets::GetSprite("pumpkin.png"));

        component->_fog->set_sprite(DanceEventAssets::Fog());
        component->_fog->set_color(UnityEngine::Color(0.45f, 0.25f, 0.6f, 0.55f));
        component->_fog->set_raycastTarget(false);
        auto fog = component->_fog->get_rectTransform();
        fog->set_anchorMin({0, 0});
        fog->set_anchorMax({1, 0});
        fog->set_pivot({0.5f, 0});
        fog->set_sizeDelta({0, 22});
        fog->set_anchoredPosition({0, 0});
        fog->SetAsFirstSibling();

        AddClick(component->_backButton, [this] {
            if (onBack) onBack();
        });
        AddClick(component->_playlistButton, [this] {
            ReeModalSystem::OpenModal<EventDetailsDialog>(LocalComponent()->_content, (Il2CppObject*)&*event);
        });

        ShowEvent();
        if (lastStatus) {
            SetData(*lastStatus);
        } else {
            SetMessage(LoadingMessage);
        }
        if (onInitialized) onInitialized();
    }

    void DanceEventPanel::SetEvent(PlatformEvent const& evt) {
        bool changed = !event || event->id != evt.id;
        event = evt;
        if (changed) {
            requestGeneration++;
            lastStatus = std::nullopt;
            SetMessage(LoadingMessage);
        }
        ShowEvent();
    }

    void DanceEventPanel::ShowEvent() {
        auto component = Usable();
        if (!component) return;
        if (event) component->_header->set_text(event->name);
        component->_playlistButton->set_interactable(event && event->downloadable);
    }

    void DanceEventPanel::Refresh() {
        if (!event) return;
        int generation = ++requestGeneration;
        DanceEventStatus::Load(event->id, std::nullopt, [this, generation](std::optional<DanceEventStatus> status) {
            if (generation != requestGeneration) return;
            if (status) {
                lastStatus = status;
                SetData(*status);
            } else if (!lastStatus) {
                SetMessage("The spirits are silent, try again later");
            }
        });
    }

    void DanceEventPanel::SetMessage(std::string const& message) {
        auto component = Usable();
        if (!component) return;
        component->_points->set_text("");
        component->_rank->set_text(message);
        component->_maps->set_text("");
    }

    static bool IsUtf8Continuation(char c) {
        return ((unsigned char)c & 0xC0) == 0x80;
    }

    static std::string Shortened(std::string const& name) {
        if (name.size() <= MaxMapNameLength) return name;
        size_t cut = MaxMapNameLength - 1;
        while (cut > 0 && IsUtf8Continuation(name[cut])) cut--;
        return name.substr(0, cut) + "...";
    }

    static std::string BeatSaverKey(std::string const& leaderboardId) {
        return leaderboardId.size() > 2 ? leaderboardId.substr(0, leaderboardId.size() - 2) : leaderboardId;
    }

    static std::string MapName(DanceEventMap const& map) {
        if (auto level = SongCore::API::Loading::GetLevelByHash(map.hash)) {
            std::string name = level->songName ? (std::string)level->songName : "";
            if (!name.empty()) return Shortened(name);
        }
        return "map " + BeatSaverKey(map.leaderboardId);
    }

    void DanceEventPanel::SetData(DanceEventStatus const& status) {
        auto component = Usable();
        if (!component) return;
        int counted = status.countedMaps;
        auto const& player = status.player;
        if (!status.eventName.empty()) component->_header->set_text(status.eventName);
        component->_points->set_text(fmt::format("{}<size=55%> / {} points</size>",
            player ? fmt::format("{:.1f}", player->points) : "0", counted * 100));
        if (!player) {
            component->_rank->set_text(fmt::format("{} dancers so far, play an event map with Dance Mode to join", status.playersCount));
            component->_maps->set_text("");
            return;
        }
        component->_rank->set_text(fmt::format(
            "#{} of {}  <color=#AAAAAA>#{} in your country</color>\n<color=#C9A0FF>{} {} above the dancing line</color>",
            player->rank, status.playersCount, player->countryRank, player->dancingMaps, player->dancingMaps == 1 ? "map" : "maps"));

        auto lines = fmt::format("<color=#AAAAAA>Your best {}</color>\n", counted);
        int shown = 0;
        for (auto const& map : player->maps) {
            if (!map.counted || shown >= counted) continue;
            shown++;
            auto mark = map.dancing ? "<color=#FF9A1F>•</color>" : "<color=#555555>•</color>";
            lines += fmt::format("{} {}  <color=#FFD700>{:.1f}</color>\n", mark, MapName(map), map.points);
        }
        component->_maps->set_text(lines);
    }

    StringW DanceEventPanel::GetContent() {
        return StringW(R"(
            <vertical id="_background" pad="1" spacing="1" pref-width="72" pref-height="82" vertical-fit="PreferredSize" child-expand-height="false" child-control-height="true" bg="round-rect-panel" bg-color="#000000aa">
                <image id="_fog" ignore-layout="true"/>
                <horizontal id="_headerBackground" pref-height="5" bg="round-rect-panel">
                    <text id="_header" text="Dance or Die!" font-size="3.5" italics="true" align="Center"/>
                </horizontal>
                <horizontal pref-height="14" spacing="1.5" child-expand-width="false" child-align="MiddleLeft">
                    <image id="_pumpkin" pref-width="13" pref-height="13" preserve-aspect="true"/>
                    <vertical pref-width="54" spacing="0" child-expand-height="false">
                        <text id="_points" text="" font-size="6" align="Left" rich-text="true" pref-height="7"/>
                        <text id="_rank" text="" font-size="2.6" align="TopLeft" rich-text="true" word-wrapping="true" pref-height="7"/>
                    </vertical>
                </horizontal>
                <horizontal pref-height="49" pad-left="2" pad-top="1">
                    <text id="_maps" text="" font-size="2.6" align="TopLeft" rich-text="true" word-wrapping="false" overflow-mode="Ellipsis"/>
                </horizontal>
                <horizontal pref-height="6" spacing="2">
                    <button id="_backButton" text="Back" pref-width="14" pref-height="6"/>
                    <button id="_playlistButton" text="Get the maps" pref-width="36" pref-height="6"/>
                </horizontal>
            </vertical>
        )");
    }
}
