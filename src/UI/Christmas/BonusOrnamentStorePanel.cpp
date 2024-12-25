#include "UI/Christmas/BonusOrnamentStorePanel.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Mathf.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "UI/Components/StaticScreen.hpp"
#include "custom-types/shared/delegate.hpp"
#include "logging.hpp"

DEFINE_TYPE(BeatLeader, BonusOrnamentStorePanel);

namespace BeatLeader {
    void BonusOrnamentStorePanel::Present() {
        screen->Present();
    }

    void BonusOrnamentStorePanel::Dismiss() {
        screen->Dismiss();
    }

    void BonusOrnamentStorePanel::Reload() {
        if (!treeStatus || treeStatus->bonusOrnaments.size() == 0) return;

        auto ornaments = treeStatus->bonusOrnaments;
        totalPages = UnityEngine::Mathf::CeilToInt(ornaments.size() / static_cast<float>(MAX_VERTICAL_CELLS));
        page = 0;
        RefreshPage();
    }

    void BonusOrnamentStorePanel::RefreshPage() {
        RefreshButtons();
        if (!treeStatus || treeStatus->bonusOrnaments.size() == 0) return;

        auto ornaments = treeStatus->bonusOrnaments;
        auto size = ornaments.size();

        for (int i = 0; i < MAX_VERTICAL_CELLS; i++) {
            int idx = i + MAX_VERTICAL_CELLS * page;
            auto cell = cells->get_Item(i);
            if (idx >= size) {
                cell->get_gameObject()->SetActive(false);
            } else {
                auto ornament = ornaments[idx];
                cell->get_gameObject()->SetActive(true);
                cell->SetBonusOrnamentStatus(ornament);
            }
        }
    }

    void BonusOrnamentStorePanel::RefreshButtons() {
        upButton->set_interactable(page > 0);
        downButton->set_interactable(page < totalPages - 1);
    }

    void BonusOrnamentStorePanel::Awake() {
        screen = get_gameObject()->AddComponent<StaticScreen*>();

        // Create vertical layout group
        auto groupObj = UnityEngine::GameObject::New_ctor("VerticalLayoutGroup");
        auto group = groupObj->AddComponent<UnityEngine::UI::VerticalLayoutGroup*>();
        group->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
        group->set_spacing(GAP_SIZE);
        group->set_childControlHeight(false);
        group->set_childForceExpandHeight(false);

        auto trans = groupObj->GetComponent<UnityEngine::RectTransform*>();
        float cellsHeight = CalcSize(MAX_VERTICAL_CELLS);
        trans->SetParent(get_transform(), false);
        trans->set_sizeDelta(UnityEngine::Vector2(CELL_SIZE, cellsHeight));

        // Create cells
        cells = System::Collections::Generic::List_1<OrnamentStoreCell*>::New_ctor();
        for (int i = 0; i < MAX_VERTICAL_CELLS; i++) {
            auto cellObj = UnityEngine::GameObject::New_ctor("OrnamentStoreCell");
            auto cell = cellObj->AddComponent<OrnamentStoreCell*>();
            cellObj->get_transform()->SetParent(group->get_transform(), false);
            cells->Add(cell);
        }

        // Create navigation buttons
        float buttonOffset = cellsHeight / 2.0f + 2.5f;

        auto upButtonObj = CreateButton(
            std::bind(&BonusOrnamentStorePanel::HandleUpButtonClicked, this), 
            upButton);
        upButtonObj->get_transform()->SetParent(get_transform(), false);
        upButtonObj->get_transform()->set_localPosition(UnityEngine::Vector3(0.0f, buttonOffset, 0.0f));
        upButtonObj->get_transform()->set_localEulerAngles(UnityEngine::Vector3(0.0f, 0.0f, 180.0f));

        auto downButtonObj = CreateButton(
            std::bind(&BonusOrnamentStorePanel::HandleDownButtonClicked, this),
            downButton);
        downButtonObj->get_transform()->SetParent(get_transform(), false);
        downButtonObj->get_transform()->set_localPosition(UnityEngine::Vector3(0.0f, -buttonOffset, 0.0f));

        API::RequestManager::AddTreeStatusRequestListener(
            std::bind(&BonusOrnamentStorePanel::HandleTreeStatusRequestState, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        // API::RequestManager::SendTreeStatusRequest();
    }

    UnityEngine::GameObject* BonusOrnamentStorePanel::CreateButton(std::function<void()> callback, UnityEngine::UI::Button*& outButton) {
        auto go = UnityEngine::GameObject::New_ctor("Button");
        
        outButton = go->AddComponent<UnityEngine::UI::Button*>();
        outButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this, callback]() {
            callback();
        }));

        auto colors = UnityEngine::UI::ColorBlock();
        colors.normalColor = UnityEngine::Color(1.0f, 1.0f, 1.0f, 0.5f);
        colors.highlightedColor = UnityEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        colors.disabledColor = UnityEngine::Color(0.0f, 0.0f, 0.0f, 0.5f);
        colors.colorMultiplier = 1.0f;
        outButton->set_colors(colors);

        auto image = go->AddComponent<UnityEngine::UI::Image*>();
        auto sprites = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Sprite*>();
        for (auto sprite : sprites) {
            if (sprite->get_name() == "ArrowIcon") {
                image->set_sprite(sprite);
                break;
            }
        }

        auto materials = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>();
        for (auto material : materials) {
            if (material->get_name() == "UINoGlow") {
                image->set_material(material);
                break;
            }
        }

        image->set_preserveAspect(true);
        outButton->set_image(image);

        auto rect = go->GetComponent<UnityEngine::RectTransform*>();
        rect->set_sizeDelta(UnityEngine::Vector2(3.0f, 5.0f));

        return go;
    }

    int BonusOrnamentStorePanel::CalcSize(int count) {
        return CELL_SIZE * count + GAP_SIZE * (count - 1);
    }

    void BonusOrnamentStorePanel::OnDestroy() {
        API::RequestManager::RemoveTreeStatusRequestListener(
            std::bind(&BonusOrnamentStorePanel::HandleTreeStatusRequestState, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void BonusOrnamentStorePanel::HandleUpButtonClicked() {
        if (page == 0) return;
        page--;
        RefreshPage();
    }

    void BonusOrnamentStorePanel::HandleDownButtonClicked() {
        if (page == totalPages - 1) return;
        page++;
        RefreshPage();
    }

    void BonusOrnamentStorePanel::HandleTreeStatusRequestState(API::RequestState state, TreeStatus* status, StringW failReason) {
        if (state != API::RequestState::Finished) return;
        treeStatus = status;
        Reload();
    }

} // namespace BeatLeader 