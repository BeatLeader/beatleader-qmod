#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/GridLayoutGroup.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "UI/Christmas/OrnamentStoreCell.hpp"
#include "Models/TreeStatus.hpp"
#include "API/RequestManager.hpp"
#include "System/Collections/Generic/List_1.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, OrnamentStorePanel, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_METHOD(void, Reload);
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_INSTANCE_FIELD(StaticScreen*, screen);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::GridLayoutGroup*, gridLayout);
    DECLARE_INSTANCE_FIELD(System::Collections::Generic::List_1<OrnamentStoreCell*>*, cells);

    void Present();
    void Dismiss();

    public:

    static constexpr int VERTICAL_CELLS = 3;
    static constexpr int HORIZONTAL_CELLS = 4;
    static constexpr int CELL_SIZE = 10;
    static constexpr int GAP_SIZE = 1;

    TreeStatus* treeStatus;

private:
    static int CalcSize(int count) {
        return CELL_SIZE * count + GAP_SIZE * (count - 1);
    }

    
    void HandleTreeStatusRequestState(API::RequestState state, TreeStatus* status, StringW failReason);
);