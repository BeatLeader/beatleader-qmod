#include "UI/MainMenu/OtherNews/EventNewsPanel.hpp"
#include "Utils/FormatUtils.hpp"
#include "UI/Abstract/ReeModalSystem.hpp"
#include "UI/MainMenu/Modals/EventDetailsDialog.hpp"

#include "Models/Paged.hpp"
#include "Models/PlatformEvent.hpp"

#include "UnityEngine/Object.hpp"
#include "UnityEngine/Transform.hpp"
#include "System/TimeSpan.hpp"
#include "Utils/WebUtils.hpp"
#include "custom-types/shared/delegate.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include <chrono>

namespace BeatLeader {

    bool EventNewsPanel::HasHeader() {
        return true;
    }

    void EventNewsPanel::OnInitialize() {
        AbstractNewsPanel::OnInitialize();
        LocalComponent()->header->SetupData("BeatLeader Events");
        LocalComponent()->_loadingIndicator->SetActive(true);
        
        WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/events", [this](long status, bool error, rapidjson::Document const& response) {
            if (status == 200 && !error) {
                auto result = Paged<PlatformEvent>(response.GetObject());
                BSML::MainThreadScheduler::Schedule([this, result] {
                    LocalComponent()->_loadingIndicator->SetActive(false);
                    if (result.data.size() > 0) {
                        LocalComponent()->_emptyText->get_gameObject()->SetActive(false);
                        PresentList(result.data);
                    } else {
                        LocalComponent()->_emptyText->get_gameObject()->SetActive(true);
                        LocalComponent()->_emptyText->set_text("There is no events");
                        DisposeList();
                    }
                });
            } else {
                BSML::MainThreadScheduler::Schedule([this] {
                    LocalComponent()->_loadingIndicator->SetActive(false);
                    LocalComponent()->_emptyText->get_gameObject()->SetActive(true);
                    LocalComponent()->_emptyText->set_text("<color=#ff8888>Failed to load");
                    DisposeList();
                });
            }
        });
    }

    void EventNewsPanel::PresentList(const std::vector<PlatformEvent>& items) {
        DisposeList();

        for (const auto& item : items) {
            auto previewPanel = FeaturedPreviewPanel::Instantiate<FeaturedPreviewPanel>(LocalComponent()->get_transform());
            previewPanel->LocalComponent()->ManualInit(LocalComponent()->_mainContainer);
            StringW bottomText;
            auto timeSpan = item.endDate - std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            
            if (timeSpan > 0) {
                bottomText = "<color=#88FF88>Ongoing!";
            } else {
                auto date = FormatUtils::GetRelativeTimeString(std::to_string(item.endDate));
                bottomText = "<color=#884444>Ended " + date;
            }

            System::Action* action = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[item, this]() {
                ReeModalSystem::OpenModal<EventDetailsDialog>(LocalComponent()->_content->get_transform(), (Il2CppObject*)&item);
            });
            previewPanel->LocalComponent()->SetupData(item.image, item.name, bottomText, "Details", action, action);
            _list->Add(previewPanel->LocalComponent());
        }

        LocalComponent()->MarkScrollbarDirty();
    }

    void EventNewsPanel::DisposeList() {
        for (int i = 0; i < _list->get_Count(); i++) {
            auto post = _list->get_Item(i);
            UnityEngine::Object::Destroy(post->get_gameObject());
        }
        _list->Clear();
        LocalComponent()->MarkScrollbarDirty();
    }

    StringW EventNewsPanel::GetContent() {
        return StringW(R"(
            <vertical id="_background" pad="1" spacing="1" vertical-fit="PreferredSize" bg="round-rect-panel" bg-color="#ffffff55">
                <macro.as-host host="header">
                    <macro.reparent transform="_uiComponent"/>
                </macro.as-host>
                <scroll-view id="_scrollView" pref-width="70" pref-height="26" pad-right="4" horizontal-fit="PreferredSize" vertical-fit="PreferredSize" bg="round-rect-panel">
                    <horizontal id="_heightWrapper" min-height="26" horizontal-fit="PreferredSize">
                        <vertical spacing="1" id="_mainContainer" vertical-fit="PreferredSize">
                            <loading-indicator id="_loadingIndicator" preserve-aspect="true" pref-height="8" pref-width="8"/>
                            <text id="_emptyText" text=""/>
                        </vertical>
                    </horizontal>
                </scroll-view>
            </vertical>
        )");
    }
} 