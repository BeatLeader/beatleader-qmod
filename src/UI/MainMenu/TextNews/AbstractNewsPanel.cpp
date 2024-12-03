#include "UI/MainMenu/TextNews/AbstractNewsPanel.hpp"
#include "UnityEngine/Transform.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, AbstractNewsPanelComponent);

namespace BeatLeader {

    void AbstractNewsPanelComponent::Awake() {
        header = NewsHeader::Instantiate<NewsHeader>(get_transform())->LocalComponent();
    }

    bool AbstractNewsPanel::HasHeader() {
        return false;
    }
    void AbstractNewsPanel::OnInitialize() {
        _list = System::Collections::Generic::List_1<UnityEngine::MonoBehaviour*>::New_ctor();

        LocalComponent()->_background->set_raycastTarget(true);
        LocalComponent()->InitializeScrollView();

        LocalComponent()->_scrollbarDirty = false;
        LocalComponent()->_skipFrames = 0;
    }

    void AbstractNewsPanelComponent::Update() {
        UpdateScrollbarIfDirty();
    }

    void AbstractNewsPanelComponent::MarkScrollbarDirty() {
        _scrollbarDirty = true;
        _skipFrames = 2;
    }

    void AbstractNewsPanelComponent::UpdateScrollbarIfDirty() {
        if (!_scrollbarDirty || _skipFrames > 0) return;
        _skipFrames--;
        _scrollbarDirty = false;
        _scrollView->SetContentSize(_heightWrapper->get_preferredHeight());
        _scrollView->ScrollTo(0.0f, false);
    }

    void AbstractNewsPanelComponent::InitializeScrollView() {
        _scrollView->_pageStepNormalizedSize = 0.5f;

        auto scrollBar = _scrollView->get_transform()->GetChild(0).cast<UnityEngine::RectTransform>();
        auto upIcon = scrollBar->GetChild(0)->GetChild(0).cast<UnityEngine::RectTransform>();
        auto downIcon = scrollBar->GetChild(1)->GetChild(0).cast<UnityEngine::RectTransform>();

        scrollBar->set_sizeDelta(UnityEngine::Vector2(4.0f, 0.0f));
        upIcon->set_anchoredPosition(UnityEngine::Vector2(-2.0f, -4.0f));
        downIcon->set_anchoredPosition(UnityEngine::Vector2(-2.0f, 4.0f));
        scrollBar->ForceUpdateRectTransforms();

        auto viewport = _scrollView->get_transform()->GetChild(1).cast<UnityEngine::RectTransform>();
        viewport->set_offsetMin(UnityEngine::Vector2(0.0f, 0.0f));
        viewport->set_offsetMax(UnityEngine::Vector2(0.0f, 0.0f));
    }
}


