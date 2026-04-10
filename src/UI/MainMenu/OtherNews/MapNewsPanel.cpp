#include "UI/MainMenu/OtherNews/MapNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/FeaturedPreviewPanel.hpp"
#include "UI/MainMenu/Modals/MapDownloadDialog.hpp"
#include "UI/MainMenu/Modals/MapPreviewDialog.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "custom-types/shared/delegate.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "Utils/WebUtils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "Models/TrendingMapData.hpp"
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
                auto result = Paged<TrendingMapData>(response.GetObject());
                BSML::MainThreadScheduler::Schedule([this, result] {
                    auto* localComponent = TryGetUsableComponent();
                    if (!localComponent || !localComponent->_loadingIndicator || !localComponent->_emptyText) {
                        return;
                    }

                    localComponent->_loadingIndicator->SetActive(false);

                    if (result.data.size() > 0) {
                        localComponent->_emptyText->get_gameObject()->SetActive(false);
                        PresentList(result.data);
                    } else {
                        localComponent->_emptyText->get_gameObject()->SetActive(true);
                        localComponent->_emptyText->set_text("There are no trending maps");
                        DisposeList();
                    }
                });
            } else {
                BSML::MainThreadScheduler::Schedule([this] {
                    auto* localComponent = TryGetUsableComponent();
                    if (!localComponent || !localComponent->_loadingIndicator || !localComponent->_emptyText) {
                        return;
                    }

                    localComponent->_loadingIndicator->SetActive(false);
                    localComponent->_emptyText->get_gameObject()->SetActive(true);
                    localComponent->_emptyText->set_text("<color=#ff8888>Failed to load");
                    DisposeList();
                });
            }
        });
    }

    void MapNewsPanel::PresentList(std::vector<TrendingMapData> const& items) {
        auto* localComponent = TryGetUsableComponent();
        if (!localComponent || !EnsureList()) {
            return;
        }

        DisposeList();

        for (int i = 0; i < items.size(); i++) {
            auto item = items[i];
            auto previewPanel = FeaturedPreviewPanel::Instantiate<FeaturedPreviewPanel>(localComponent->get_transform());
            if (!previewPanel) {
                continue;
            }

            auto panelComponent = previewPanel->LocalComponent();
            if (!panelComponent) {
                continue;
            }

            panelComponent->ManualInit(localComponent->_mainContainer);
            
            auto buttonAction = custom_types::MakeDelegate<System::Action*>(
                (std::function<void()>)[item, this] {
                    auto* localComponent = this->TryGetUsableComponent();
                    if (!localComponent || !localComponent->_content) {
                        return;
                    }

                    MapDownloadDialog::OpenSongOrDownloadDialog(item.song, localComponent->_content->get_transform());
                }
            );

            auto backgroundAction = custom_types::MakeDelegate<System::Action*>(
                (std::function<void()>)[item, this] {
                    auto* localComponent = this->TryGetUsableComponent();
                    if (!localComponent || !localComponent->_content) {
                        return;
                    }

                    MapPreviewDialog::OpenSongOrDownloadDialog(item, localComponent->_content->get_transform());
                }
            );

            panelComponent->SetupData(
                item.song.coverImage,
                item.song.name,
                item.song.mapper,
                "Play",
                buttonAction,
                backgroundAction
            );

            _list->Add(panelComponent);
        }

        auto* refreshedComponent = TryGetUsableComponent();
        if (refreshedComponent) {
            refreshedComponent->MarkScrollbarDirty();
        }
    }

    void MapNewsPanel::DisposeList() {
        if (!_list) {
            return;
        }

        for (int i = 0; i < _list->get_Count(); i++) {
            auto post = _list->get_Item(i);
            if (post != nullptr && post->get_gameObject() != nullptr) {
                UnityEngine::Object::Destroy(post->get_gameObject());
            }
        }

        _list->Clear();
        auto* localComponent = TryGetUsableComponent();
        if (localComponent) {
            localComponent->MarkScrollbarDirty();
        }
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
