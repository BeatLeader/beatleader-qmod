#include "UI/MainMenu/TextNews/TextNewsPanel.hpp"
#include "UI/MainMenu/TextNews/TextNewsPostPanel.hpp"

#include "UnityEngine/GameObject.hpp"

#include "Utils/WebUtils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "main.hpp"


namespace BeatLeader {

    void TextNewsPanel::OnInitialize() {
        AbstractNewsPanel::OnInitialize();
        LocalComponent()->_loadingIndicator->SetActive(true);

        WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/news", [this](long status, bool error, rapidjson::Document const& response) {
            if (status == 200 && !error) {
                auto news = Paged<NewsPost>(response.GetObject());
                BSML::MainThreadScheduler::Schedule([this, news] {
                    ShowNews(news);
                });
            } else {
                BSML::MainThreadScheduler::Schedule([this] {
                    ShowError();
                });
            }
        });
    }

    void TextNewsPanel::ShowNews(Paged<NewsPost> news) {
        LocalComponent()->_loadingIndicator->SetActive(false);
        if (news.data.size() > 0) {
            LocalComponent()->_emptyText->get_gameObject()->SetActive(false);
            PresentList(news.data);
        } else {
            LocalComponent()->_emptyText->get_gameObject()->SetActive(true);
            LocalComponent()->_emptyText->set_text("There is no news");
            DisposeList();
        }
    }

    void TextNewsPanel::ShowError() {
        LocalComponent()->_loadingIndicator->SetActive(false);
        LocalComponent()->_emptyText->get_gameObject()->SetActive(true);
        LocalComponent()->_emptyText->set_text("<color=#ff8888>Failed to load");
        DisposeList();
    }   

    void TextNewsPanel::PresentList(std::vector<NewsPost> const& items) {
        DisposeList();

        for (int i = 0; i < items.size(); i++) {
            auto item = items[i];
            auto postPanel = TextNewsPostPanel::Instantiate<TextNewsPostPanel>(LocalComponent()->get_transform());
            postPanel->LocalComponent()->ManualInit(LocalComponent()->_mainContainer);
            postPanel->SetupData(item);
            _list->Add(postPanel->LocalComponent());
        }

        LocalComponent()->MarkScrollbarDirty();
    }

    void TextNewsPanel::DisposeList() {
        for (int i = 0; i < _list->get_Count(); i++) {
            auto component = _list->get_Item(i);
            UnityEngine::Object::Destroy(component->get_gameObject());
        }
        _list->Clear();
        LocalComponent()->MarkScrollbarDirty();
    }

    StringW TextNewsPanel::GetContent() {
        return StringW(R"(
            <vertical id="_background" pad="1" spacing="1" vertical-fit="PreferredSize" bg="round-rect-panel" bg-color="#ffffff55">
                <scroll-view id="_scrollView" pref-width="70" pref-height="80" pad-right="4" horizontal-fit="PreferredSize" vertical-fit="PreferredSize" bg="round-rect-panel">
                    <horizontal id="_heightWrapper" min-height="81" horizontal-fit="PreferredSize">
                        <vertical spacing="3" id="_mainContainer" vertical-fit="PreferredSize">
                            <loading-indicator id="_loadingIndicator" preserve-aspect="true" pref-height="8" pref-width="8"/>
                            <text id="_emptyText" text=""/>
                        </vertical>
                    </horizontal>
                </scroll-view>
            </vertical>
        )");
    }
} 