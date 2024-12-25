#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "UI/Christmas/OrnamentStoreCell.hpp"
#include "Models/TreeStatus.hpp"
#include "API/RequestManager.hpp"

#include "System/Collections/Generic/List_1.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, BonusOrnamentStorePanel, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(StaticScreen*, screen);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, upButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, downButton);
    DECLARE_INSTANCE_FIELD(System::Collections::Generic::List_1<OrnamentStoreCell*>*, cells);
    DECLARE_INSTANCE_FIELD(int, totalPages);
    DECLARE_INSTANCE_FIELD(int, page);

    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

public:
    TreeStatus* treeStatus;

private:
    static constexpr int MAX_VERTICAL_CELLS = 3;
    static constexpr int CELL_SIZE = 10;
    static constexpr int GAP_SIZE = 1;

    void Reload();
    void RefreshPage();
    void RefreshButtons();
    void HandleUpButtonClicked();
    void HandleDownButtonClicked();
    void HandleTreeStatusRequestState(API::RequestState state, TreeStatus* status, StringW failReason);
    UnityEngine::GameObject* CreateButton(std::function<void()> callback, UnityEngine::UI::Button*& outButton);
    static int CalcSize(int count);
)