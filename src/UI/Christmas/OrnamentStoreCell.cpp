#include "UI/Christmas/OrnamentStoreCell.hpp"
#include "UI/Christmas/ChristmasTreeOrnament.hpp"
#include "UI/Christmas/ChristmasOrnamentLoader.hpp"
#include "UI/Christmas/ChristmasTreeManager.hpp"
#include "UI/MainMenu/Modals/MapDownloadDialog.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "Utils/WebUtils.hpp"
#include "BSML/shared/Helpers/getters.hpp"
#include "Assets/Sprites.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, OrnamentStoreCell);

namespace BeatLeader {

    void OrnamentStoreCell::Awake() {
        // Setup background image
        auto image = get_gameObject()->AddComponent<UnityEngine::UI::Image*>();
        image->set_sprite(BundleLoader::bundle->OrnamentCellBG);
        image->set_pixelsPerUnitMultiplier(5.0f);
        image->set_type(UnityEngine::UI::Image::Type::Sliced);
        image->set_color(UnityEngine::Color::get_black());

        // Setup hover hint
        hint = BSML::Helpers::GetDiContainer()->InstantiateComponent<HMUI::HoverHint*>(get_gameObject());

        // Setup text
        auto textObj = UnityEngine::GameObject::New_ctor("Text");
        text = textObj->AddComponent<TMPro::TextMeshProUGUI*>();
        text->set_alignment(TMPro::TextAlignmentOptions::Center);
        text->set_fontSize(4.0f);
        text->set_color(UnityEngine::Color::get_red());

        auto trans = textObj->GetComponent<UnityEngine::RectTransform*>();
        trans->SetParent(get_transform(), false);
        trans->set_sizeDelta(UnityEngine::Vector2(CELL_SIZE, CELL_SIZE));

        trans = GetComponent<UnityEngine::RectTransform*>();
        trans->set_sizeDelta(UnityEngine::Vector2(CELL_SIZE, CELL_SIZE));
    }

    void OrnamentStoreCell::SetOpeningDayIndex(int dayIndex) {
        text->set_text("Dec " + std::to_string(19 + dayIndex));
        if (dayIndex == 6) {
            text->set_color(UnityEngine::Color::get_yellow());
        }
    }

    void OrnamentStoreCell::SetOrnamentStatus(std::optional<DailyTreeStatus> status) {
        if (!status) {
            return;
        }

        if (status->score) {
            bundleId = status->bundleId;
            ReloadNextInstance(false);
            if (previewImage) {
                previewImage->get_gameObject()->SetActive(false);
            }
        } else {
            song = &status->song;
            if (!previewImage) {

                previewImage = ::BSML::Lite::CreateClickableImage(get_transform(), BundleLoader::bundle->overview1Icon, [this](){
                    if (song) {
                        MapDownloadDialog::OpenSongOrDownloadDialog(*song, get_transform());
                    }
                }, UnityEngine::Vector2(0, 0), UnityEngine::Vector2(CELL_SIZE, CELL_SIZE));
            }
            previewImage->get_gameObject()->SetActive(true);
            hint->set_text("Pass " + status->song.name + " from " + status->song.mapper + " to unlock this ornament");

            Sprites::get_Icon("https://cdn.assets.beatleader.xyz/project_tree_ornament_" + std::to_string(status->bundleId) + "_preview.png", [this](UnityEngine::Sprite* sprite) {
                if (previewImage) {
                    previewImage->set_sprite(sprite);
                }
            });
        }
    }

    void OrnamentStoreCell::SetBonusOrnamentStatus(std::optional<BonusOrnament> status) {
        if (!status) {
            return;
        }

        bundleId = status->bundleId;
        ReloadNextInstance(true);
        if (previewImage) {
            previewImage->get_gameObject()->SetActive(false);
        }
        if (!status->description.empty()) {
            hint->set_text(status->description);
        }
    }

    void OrnamentStoreCell::ReloadNextInstance(bool despawn) {
        if (despawn && previewInstance) {
            UnityEngine::Object::Destroy(previewInstance->get_gameObject());
        }

        BeatLeaderLogger.info("Loading ornament {}", bundleId);
        auto tree = ChristmasTreeManager::christmasTree;

        ChristmasOrnamentLoader::Spawn(bundleId, tree, get_transform(), 
            [this](ChristmasTreeOrnament* ornament) {
                BeatLeaderLogger.info("Loaded ornament");
                previewInstance = ornament;
                previewInstance->get_transform()->set_localPosition(UnityEngine::Vector3(0.0f, 2.5f, 0.0f));
                previewInstance->get_transform()->set_localScale(UnityEngine::Vector3(50.0f, 50.0f, 50.0f));
                previewInstance->_grabbedAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
                    previewInstance->_grabbedAction = nullptr;
                    ReloadNextInstance(false);
                });
                previewInstance->canGrab = true;
            });
    }

    void OrnamentStoreCell::HandlePreviewOrnamentGrabbed(ChristmasTreeOrnament* ornament) {
        // TODO: Remove ornament grabbed event
        ReloadNextInstance(false);
    }

} // namespace BeatLeader 