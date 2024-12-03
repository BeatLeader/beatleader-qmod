#include "UI/MainMenu/OtherNews/MapNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/FeaturedPreviewPanel.hpp"
#include "UI/MainMenu/Modals/MapDownloadDialog.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "custom-types/shared/delegate.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Utils/WebUtils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "Models/MapData.hpp"
#include "Models/Paged.hpp"

namespace BeatLeader {
    bool MapNewsPanel::HasHeader() {
        return true;
    }

    void MapNewsPanel::OnInitialize() {
        AbstractNewsPanel::OnInitialize();
        
        LocalComponent()->header->SetupData("Trending Maps");
        LocalComponent()->_loadingIndicator->SetActive(true);
        
        WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/maps/trending", [this](long status, bool error, rapidjson::Document const& response) {
            if (status == 200 && !error) {
                auto result = Paged<MapData>(response.GetObject());
                BSML::MainThreadScheduler::Schedule([this, result] {
                    LocalComponent()->_loadingIndicator->SetActive(false);

                    if (result.data.size() > 0) {
                        LocalComponent()->_emptyText->get_gameObject()->SetActive(false);
                        PresentList(result.data);
                    } else {
                        LocalComponent()->_emptyText->get_gameObject()->SetActive(true);
                        LocalComponent()->_emptyText->set_text("There are no trending maps");
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

    void MapNewsPanel::PresentList(std::vector<MapData> const& items) {
        DisposeList();

        for (int i = 0; i < items.size(); i++) {
            auto item = items[i];
            auto previewPanel = FeaturedPreviewPanel::Instantiate<FeaturedPreviewPanel>(LocalComponent()->get_transform());
            previewPanel->LocalComponent()->ManualInit(LocalComponent()->_mainContainer);
            auto panelComponent = previewPanel->LocalComponent();
            
            auto buttonAction = custom_types::MakeDelegate<System::Action*>(
                (std::function<void()>)[item, this] {
                    MapDownloadDialog::OpenSongOrDownloadDialog(item.song, this->LocalComponent()->_content->get_transform());
                }
            );

            panelComponent->SetupData(
                item.song.coverImage,
                item.song.name,
                item.song.mapper,
                "Play",
                buttonAction
            );

            _list->Add(panelComponent);
        }

        LocalComponent()->MarkScrollbarDirty();
    }

    void MapNewsPanel::DisposeList() {
        for (int i = 0; i < _list->get_Count(); i++) {
            auto post = _list->get_Item(i);
            UnityEngine::Object::Destroy(post->get_gameObject());
        }

        _list->Clear();
        LocalComponent()->MarkScrollbarDirty();
    }

    StringW MapNewsPanel::GetContent() {
        return StringW(R"(
            <vertical id="_background" pad="1" spacing="1" vertical-fit="PreferredSize" bg="round-rect-panel" bg-color="#ffffff55">
                <macro.as-host host="header">
                    <macro.reparent transform="_uiComponent"/>
                </macro.as-host>
                <scroll-view id="_scrollView" pref-width="70" pref-height="39" pad-right="4" horizontal-fit="PreferredSize" vertical-fit="PreferredSize" bg="round-rect-panel">
                    <horizontal id="_heightWrapper" min-height="39" horizontal-fit="PreferredSize">
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