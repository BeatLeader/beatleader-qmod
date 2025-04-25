#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/ScrollRect.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "HMUI/ScrollView.hpp"
#include "HMUI/ImageView.hpp"
#include "UI/MainMenu/OtherNews/NewsHeader.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, AbstractNewsPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(HMUI::ScrollView*, _scrollView);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, _mainContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::ScrollRect*, _scrollRect);
    DECLARE_INSTANCE_FIELD(BeatLeader::NewsHeaderComponent*, header);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::HorizontalLayoutGroup*, _heightWrapper);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, _background);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _emptyText);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingIndicator);

    DECLARE_INSTANCE_FIELD(bool, _scrollbarDirty);
    DECLARE_INSTANCE_FIELD(int, _skipFrames);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, MarkScrollbarDirty);
    DECLARE_INSTANCE_METHOD(void, UpdateScrollbarIfDirty);
    DECLARE_INSTANCE_METHOD(void, InitializeScrollView);
};

namespace BeatLeader {

class AbstractNewsPanel : public ReeUIComponentV2<AbstractNewsPanelComponent*> {
public:
    void OnInitialize();
    virtual bool HasHeader();
    System::Collections::Generic::List_1<UnityEngine::MonoBehaviour*>* _list;
};

} // namespace BeatLeader

