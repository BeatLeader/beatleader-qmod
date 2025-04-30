#include "UI/MainMenu/Modals/MapPreviewDialog.hpp"
#include "Utils/WebUtils.hpp"
#include "Utils/FileManager.hpp"
#include "GlobalNamespace/LevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/SoloFreePlayFlowCoordinator.hpp"
#include "UnityEngine/AsyncOperation.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/AudioType.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Networking/UnityWebRequest.hpp"
#include "UnityEngine/Networking/DownloadHandlerAudioClip.hpp"
#include "UnityEngine/Networking/UnityWebRequestMultimedia.hpp"

#include "System/Action_1.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/IO/Path.hpp"
#include "HMUI/NoTransitionsButton.hpp"
#include "UI/Abstract/ReeModalSystem.hpp"
#include "custom-types/shared/delegate.hpp"
#include "songcore/shared/SongCore.hpp"
#include "Utils/PlaylistSynchronizer.hpp"
#include "Utils/StringUtils.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "include/Assets/Sprites.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(BeatLeader, MapPreviewDialogComponent);

namespace BeatLeader {

    void MapPreviewDialog::OnInitialize() {
        AbstractReeModal::OnInitialize();

        // add actions to buttons
        LocalComponent()->_playButton->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>((std::function<void()>)[this]() {
            MapPreviewDialog::HandlePlayButtonClicked();
        }));
    }

    void MapPreviewDialog::OnContextChanged() {
        auto array = UnityEngine::Resources::FindObjectsOfTypeAll<SongPreviewPlayer*>();
        if (array.size() > 0) {
            songPreviewPlayer = array[0];
        }

        LocalComponent()->_mapName->set_text(context->song.name);
        LocalComponent()->_songAuthor->set_text(context->song.author);
        LocalComponent()->_mapper->set_text("Mapped by " + context->song.mapper);
        LocalComponent()->_description->set_text(context->description);
        LocalComponent()->_trendingValue->set_text(context->trendingValue);

        LoadCoverImage();
    }

    void MapPreviewDialog::LoadCoverImage() {
        if (context->song.coverImage.empty()) return;

        Sprites::get_Icon(context->song.coverImage, [this](UnityEngine::Sprite* sprite) {
            if (sprite != nullptr) {
                BSML::MainThreadScheduler::Schedule([this, sprite] {
                    LocalComponent()->_coverImage->set_sprite(sprite);
                });
            }
        });
    }

    void MapPreviewDialog::LoadAndPlayPreview() {
        if (context->song.hash.empty()) return;

        auto previewUrl = "https://eu.cdn.beatsaver.com/" + toLower(context->song.hash) + ".mp3";
        
        auto request = UnityEngine::Networking::UnityWebRequestMultimedia::GetAudioClip(previewUrl, UnityEngine::AudioType::MPEG);

        request->SendWebRequest()->add_completed(custom_types::MakeDelegate<System::Action_1<UnityEngine::AsyncOperation*>*>(std::function<void(UnityEngine::AsyncOperation*)>([this, request](UnityEngine::AsyncOperation* active) {
            if (request->get_result() == UnityEngine::Networking::UnityWebRequest::Result::Success) {
                previewClip = UnityEngine::Networking::DownloadHandlerAudioClip::GetContent(request);
                if (previewClip != nullptr && songPreviewPlayer != nullptr) {
                    songPreviewPlayer->CrossfadeTo(previewClip, 0, 1, previewClip->length, nullptr);
                }
            }
        })));
    }

    void MapPreviewDialog::OnResume() {
        LocalComponent()->_loadingContainer->SetActive(false);
        LocalComponent()->_finishedContainer->SetActive(true);
        LocalComponent()->_finishedText->set_text("");
        LocalComponent()->_playButton->set_interactable(true);
        offClickCloses = true;

        LoadAndPlayPreview();
    }

    void MapPreviewDialog::OnClose() {
        if (songPreviewPlayer != nullptr) {
            songPreviewPlayer->CrossfadeToDefault();
        }
    }

    void MapPreviewDialog::HandlePlayButtonClicked() {
        LocalComponent()->_playButton->set_interactable(false);
        LocalComponent()->_loadingContainer->SetActive(true);
        LocalComponent()->_finishedContainer->SetActive(false);
        offClickCloses = false;

        PlaylistSynchronizer::DownloadBeatmap(context->song.downloadUrl, context->song.hash, 0, [this](bool success) {
            BSML::MainThreadScheduler::ScheduleAfterTime(2, [this, success] {
                auto map = FetchMap(context->song);
                if (map != nullptr) {
                    OpenMap(map);
                    return;
                }

                LocalComponent()->_finishedText->set_text(success ? "Download has finished" : "Download has failed!");
                LocalComponent()->_loadingContainer->SetActive(false);
                LocalComponent()->_finishedContainer->SetActive(true);
                LocalComponent()->_playButton->set_interactable(true);
                offClickCloses = true;
            });
        });
    }

    void MapPreviewDialog::HandleCloseButtonClicked() {
        Close();
    }

    void MapPreviewDialog::OpenSongOrDownloadDialog(TrendingMapData mapDetail, Transform* screenChild) {
        auto map = FetchMap(mapDetail.song);
        if (map != nullptr) {
            OpenMap(map);
        } else {
            ReeModalSystem::OpenModal<MapPreviewDialog>(screenChild, (Il2CppObject*)&mapDetail);
        }
    }

    BeatmapLevel* MapPreviewDialog::FetchMap(MapDetail mapDetail) {
        return SongCore::API::Loading::GetLevelByHash(mapDetail.hash);
    }

    void MapPreviewDialog::OpenMap(BeatmapLevel* map) {
        if (map == nullptr) return;

        auto customLevelsPack = SongCore::API::Loading::GetCustomLevelPack();
        if (customLevelsPack == nullptr || customLevelsPack->____allBeatmapLevels->Count == 0) return;

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

        auto soloFreePlayFlowCoordinator = UnityEngine::Object::FindObjectOfType<SoloFreePlayFlowCoordinator*>();
        soloFreePlayFlowCoordinator->Setup(state);

        SafePtrUnity<UnityEngine::GameObject> songSelectButton = UnityEngine::GameObject::Find("SoloButton").unsafePtr();
        if (!songSelectButton) {
            songSelectButton = UnityEngine::GameObject::Find("Wrapper/BeatmapWithModifiers/BeatmapSelection/EditButton");
        }
        if (!songSelectButton) return;
        
        songSelectButton->GetComponent<HMUI::NoTransitionsButton*>()->Press();
    }

    StringW MapPreviewDialog::GetContent() {
        return StringW(R"(
            <horizontal pad="2" bg="round-rect-panel">
                <vertical pad="2" spacing="2" horizontal-fit="PreferredSize" vertical-fit="PreferredSize">
                    <horizontal bg="panel-top" pad="2">
                        <text id="_description" text="" align="Left"/>
                        <text id="_trendingValue" text="" align="Right" color="#11BBDD"/>
                    </horizontal>
                    
                    <horizontal>
                        <image id="_coverImage" pref-width="30" pref-height="30"/>
                        <vertical pad-left="4" spacing="1">
                            <text id="_mapName" text="" align="Left" font-size="5"/>
                            <text id="_songAuthor" text="" align="Left" font-size="4"/>
                            <text id="_mapper" text="" align="Left" font-size="3.5" color="#888888"/>
                        </vertical>
                    </horizontal>
                    
                    <vertical id="_loadingContainer" spacing="2" vertical-fit="PreferredSize">
                        <loading-indicator preserve-aspect="true" pref-height="8" pref-width="8"/>
                        <text text="Loading..."/>
                    </vertical>
                    
                    <vertical id="_finishedContainer" spacing="2">
                        <text id="_finishedText" text="" align="Center"/>
                        <button id="_playButton" text="Play"/>
                    </vertical>
                </vertical>
            </horizontal>
        )");
    }
} 