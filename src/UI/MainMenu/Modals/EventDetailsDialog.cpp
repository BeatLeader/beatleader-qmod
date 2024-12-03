#include "UI/MainMenu/Modals/EventDetailsDialog.hpp"

#include "GlobalNamespace/LevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"

#include "Utils/PlaylistSynchronizer.hpp"
#include "Utils/WebUtils.hpp"
#include "custom-types/shared/delegate.hpp"
#include "playlistcore/shared/PlaylistCore.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "HMUI/NoTransitionsButton.hpp"
#include "main.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(BeatLeader, EventDetailsDialogComponent);

namespace BeatLeader {

    void EventDetailsDialog::OnInitialize() {
        AbstractReeModal::OnInitialize();

        // add action to button
        LocalComponent()->_downloadButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this]() {
            EventDetailsDialog::HandleDownloadButtonClicked();
        }));
    }

    void EventDetailsDialog::OnContextChanged() {
        LocalComponent()->_eventDescription->SetText(context->description, true);
        LocalComponent()->_downloadButton->set_interactable(context->downloadable);
    }

    void EventDetailsDialog::OnResume() {
        LocalComponent()->_eventContainer->SetActive(true);
        LocalComponent()->_loadingContainer->SetActive(false);
    }

    void OpenPlaylist(PlaylistCore::Playlist* playlist) {
        GlobalNamespace::BeatmapLevelPack* playlistCS = playlist->playlistCS;
        GlobalNamespace::BeatmapKey beatmapKey;
        auto levelCategory = System::Nullable_1<SelectLevelCategoryViewController::LevelCategory>();
        levelCategory.value = SelectLevelCategoryViewController::LevelCategory::CustomSongs;
        levelCategory.hasValue = true;

        auto state = LevelSelectionFlowCoordinator::State::New_ctor(
            levelCategory,
            playlistCS,
            ByRef<GlobalNamespace::BeatmapKey>(&beatmapKey),
            nullptr);
        UnityEngine::GameObject::FindObjectOfType<SoloFreePlayFlowCoordinator*>()->Setup(state);
        auto soloButton = UnityEngine::GameObject::Find(StringW("SoloButton"));
        if (!soloButton) {
            soloButton = UnityEngine::GameObject::Find(StringW("Wrapper/BeatmapWithModifiers/BeatmapSelection/EditButton"));
        }
        if (soloButton) {
            auto button = soloButton->GetComponent<HMUI::NoTransitionsButton*>();
            if (button) {
                button->get_onClick()->Invoke();
            }
        }
    }

    PlaylistCore::Playlist* GetPlaylistByName(std::string filename) {
        auto loadedPlaylists = PlaylistCore::GetLoadedPlaylists();
        for (auto& loadedPlaylist : loadedPlaylists) {
            if (loadedPlaylist->path.ends_with(filename)) {
                return loadedPlaylist;
            }
        }
        return nullptr;
    }

    void EventDetailsDialog::HandleDownloadButtonClicked() {
        std::string filename = context->name;
        std::replace(filename.begin(), filename.end(), ' ', '_');
        auto playlist = GetPlaylistByName(filename + ".bplist");
        
        if (playlist == nullptr) {
            
            LocalComponent()->_eventContainer->SetActive(false);
            LocalComponent()->_loadingContainer->SetActive(true);
            offClickCloses = false;

            PlaylistSynchronizer::InstallPlaylist(WebUtils::API_URL + "playlist/" + std::to_string(context->playlistId), filename, [this, filename](bool success) {
                if (success) {
                    BSML::MainThreadScheduler::ScheduleAfterTime(5, [this, filename] {
                        auto playlist = GetPlaylistByName(filename + ".bplist");
                        if (playlist) {
                            OpenPlaylist(playlist);
                        } else {
                            LocalComponent()->_loadingContainer->SetActive(false);
                            LocalComponent()->_eventContainer->SetActive(true);
                            LocalComponent()->_eventDescription->SetText("Failed to download playlist", true);
                        }
                    });
                }
            });
        } else {
            OpenPlaylist(playlist);
        }
    }

    StringW EventDetailsDialog::GetContent() {
        return StringW(R"(
            <horizontal pad="2" bg="round-rect-panel">
                <vertical id="_loadingContainer" spacing="2" vertical-fit="PreferredSize">
                    <loading-indicator preserve-aspect="true" pref-height="8" pref-width="8"/>
                    <text text="Loading..."/>
                </vertical>
                <vertical id="_eventContainer" vertical-fit="PreferredSize">
                    <text id="_eventDescription" text="" font-size="3" align="Center"/>
                    <button text="Download Playlist" id="_downloadButton"/>
                </vertical>
            </horizontal>
        )");
    }
} 