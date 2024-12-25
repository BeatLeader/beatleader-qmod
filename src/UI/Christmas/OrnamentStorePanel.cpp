#include "UI/Christmas/OrnamentStorePanel.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "main.hpp"

DEFINE_TYPE(BeatLeader, OrnamentStorePanel);

namespace BeatLeader {

    void OrnamentStorePanel::Present() {
        screen->Present();
    }

    void OrnamentStorePanel::Dismiss() {
        screen->Dismiss();
    }

    void OrnamentStorePanel::Reload() {
        if (!treeStatus) return;

        auto ornaments = treeStatus->previousDays;
        auto size = ornaments.size();

        for (int i = 0; i < VERTICAL_CELLS * HORIZONTAL_CELLS; i++) {
            auto cell = cells->get_Item(i);

            if (i <= size) {
                auto ornament = i == size ? treeStatus->today : ornaments[i];
                cell->SetOrnamentStatus(ornament);
            } else {
                cell->SetOpeningDayIndex(i);
            }
        }
    }

    void OrnamentStorePanel::Awake() {
        screen = get_gameObject()->AddComponent<StaticScreen*>();

        // Create grid layout
        auto gridObj = UnityEngine::GameObject::New_ctor("GridLayoutGroup");
        gridLayout = gridObj->AddComponent<UnityEngine::UI::GridLayoutGroup*>();
        gridLayout->set_cellSize(UnityEngine::Vector2(CELL_SIZE, CELL_SIZE));
        gridLayout->set_spacing(UnityEngine::Vector2(GAP_SIZE, GAP_SIZE));
        gridLayout->set_constraint(UnityEngine::UI::GridLayoutGroup::Constraint::FixedColumnCount);
        gridLayout->set_constraintCount(HORIZONTAL_CELLS);
        gridLayout->set_startCorner(UnityEngine::UI::GridLayoutGroup::Corner::UpperLeft);

        auto trans = gridObj->GetComponent<UnityEngine::RectTransform*>();
        trans->SetParent(get_transform(), false);
        trans->set_sizeDelta(UnityEngine::Vector2(CalcSize(HORIZONTAL_CELLS), CalcSize(VERTICAL_CELLS)));

        // Create cells
        cells = System::Collections::Generic::List_1<OrnamentStoreCell*>::New_ctor();
        for (int i = 0; i < VERTICAL_CELLS * HORIZONTAL_CELLS; i++) {
            auto cellObj = UnityEngine::GameObject::New_ctor("OrnamentStoreCell");
            auto cell = cellObj->AddComponent<OrnamentStoreCell*>();
            cellObj->get_transform()->SetParent(gridLayout->get_transform(), false);
            cells->Add(cell);
        }

        API::RequestManager::AddTreeStatusRequestListener(
            std::bind(&OrnamentStorePanel::HandleTreeStatusRequestState, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        API::RequestManager::SendTreeStatusRequest();
    }

    void OrnamentStorePanel::OnDestroy() {
        API::RequestManager::RemoveTreeStatusRequestListener(
            std::bind(&OrnamentStorePanel::HandleTreeStatusRequestState, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void OrnamentStorePanel::HandleTreeStatusRequestState(API::RequestState state, TreeStatus* status, StringW failReason) {
        if (state != API::RequestState::Finished) return;
        treeStatus = status;
        Reload();
    }

} // namespace BeatLeader 