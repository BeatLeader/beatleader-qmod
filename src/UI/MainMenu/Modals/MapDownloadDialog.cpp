#include "UI/MainMenu/Modals/MapDownloadDialog.hpp"
#include "Utils/WebUtils.hpp"
#include "Utils/FileManager.hpp"
#include "GlobalNamespace/LevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/IO/Path.hpp"
#include "HMUI/NoTransitionsButton.hpp"
#include "UI/Abstract/ReeModalSystem.hpp"
#include "custom-types/shared/delegate.hpp"
#include "songcore/shared/SongCore.hpp"
#include "Utils/PlaylistSynchronizer.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(BeatLeader, MapDownloadDialogComponent);

namespace BeatLeader {

    void MapDownloadDialog::OnInitialize() {
        AbstractReeModal::OnInitialize();

        // add action to button
        LocalComponent()->_okButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this]() {
            MapDownloadDialog::HandleOkButtonClicked();
        }));
    }

    void MapDownloadDialog::OnContextChanged() {
    }

    void MapDownloadDialog::OnResume() {
        LocalComponent()->_loadingContainer->SetActive(true);
        LocalComponent()->_finishedContainer->SetActive(false);
        offClickCloses = false;
    }

    void MapDownloadDialog::HandleOkButtonClicked() {
        Close();
    }

    void MapDownloadDialog::OpenSongOrDownloadDialog(MapDetail mapDetail, Transform* screenChild) {
        auto map = FetchMap(mapDetail);
        if (map != nullptr) {
            OpenMap(map);
        } else {
            MapDownloadDialog* modal = static_cast<MapDownloadDialog*>(ReeModalSystem::OpenModal<MapDownloadDialog>(screenChild, (Il2CppObject*)&mapDetail));
            PlaylistSynchronizer::DownloadBeatmap(mapDetail.downloadUrl, mapDetail.hash, 0, [modal, mapDetail](bool success) {
                BSML::MainThreadScheduler::ScheduleAfterTime(5, [modal, success, mapDetail] {
                    auto map = FetchMap(mapDetail);
                    if (map != nullptr) {
                        // modal->Close();
                        OpenMap(map);
                    } else {
                        modal->LocalComponent()->_finishedText->set_text(success ? "Download has finished" : "Download has failed!");
                        modal->LocalComponent()->_loadingContainer->SetActive(false);
                        modal->LocalComponent()->_finishedContainer->SetActive(true);
                        modal->LocalComponent()->_okButton->get_gameObject()->SetActive(true);
                    }
                });
            });
        }
    }

    BeatmapLevel* MapDownloadDialog::FetchMap(MapDetail mapDetail) {
        return SongCore::API::Loading::GetLevelByHash(mapDetail.hash);
    }

    void MapDownloadDialog::OpenMap(BeatmapLevel* map) {
        if (map == nullptr) {
            return;
        }

        auto customLevelsPack = SongCore::API::Loading::GetCustomLevelPack();
        if (customLevelsPack == nullptr) {
            return;
        }
        if (customLevelsPack->___beatmapLevels->get_Length() == 0) {
            return;
        }

        auto category = SelectLevelCategoryViewController::LevelCategory(
                SelectLevelCategoryViewController::LevelCategory::All);

        auto levelCategory = System::Nullable_1<SelectLevelCategoryViewController::LevelCategory>();
        levelCategory.value = category;
        levelCategory.hasValue = true;

        auto state = LevelSelectionFlowCoordinator::State::New_ctor(
                customLevelsPack,
                map
        );

        state->___levelCategory = levelCategory;

        auto soloFreePlayFlowCoordinator = UnityEngine::Object::FindObjectOfType<SoloFreePlayFlowCoordinator *>();
        soloFreePlayFlowCoordinator->Setup(state);

        SafePtrUnity<UnityEngine::GameObject> songSelectButton = UnityEngine::GameObject::Find("SoloButton").unsafePtr();
        if (!songSelectButton) {
            songSelectButton = UnityEngine::GameObject::Find("Wrapper/BeatmapWithModifiers/BeatmapSelection/EditButton");
        }
        if (!songSelectButton) {
            return;
        }
        songSelectButton->GetComponent<HMUI::NoTransitionsButton *>()->Press();
    }

    StringW MapDownloadDialog::GetContent() {
        return StringW(R"(
            <horizontal pad="2" bg="round-rect-panel">
                <vertical id="_loadingContainer" spacing="2" vertical-fit="PreferredSize">
                    <loading-indicator preserve-aspect="true" pref-height="8" pref-width="8"/>
                    <text text="Loading..."/>
                </vertical>
                <vertical id="_finishedContainer">
                    <text id="_finishedText" text="" align="Bottom"/>
                    <button text="Ok" id="_okButton"/>
                </vertical>
            </horizontal>
        )");
    }
} 