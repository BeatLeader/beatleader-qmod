#pragma once

#include "HMUI/ModalView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "Models/TrendingMapData.hpp"
#include "UI/Abstract/AbstractReeModal.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, MapPreviewDialogComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _finishedContainer);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _finishedText);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _mapName);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _songAuthor);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _mapper);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _description);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _trendingValue);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Image*, _coverImage);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _playButton);
};

namespace BeatLeader {

class MapPreviewDialog : public AbstractReeModal<MapPreviewDialogComponent*> {
public:
    using ComponentType = MapPreviewDialogComponent*;
    
    bool mapDownloaded;
    TrendingMapData* context;
    GlobalNamespace::SongPreviewPlayer* songPreviewPlayer;
    UnityEngine::AudioClip* previewClip;

    void OnContextChanged() override;
    void OnResume() override;
    void OnClose() override;
    void SetContext(Il2CppObject* context) override {
        this->context = reinterpret_cast<TrendingMapData*>(context);
        OnContextChanged();
    }
    void HandlePlayButtonClicked();
    void HandleCloseButtonClicked();

    void LoadCoverImage();
    void LoadAndPlayPreview();

    // Override from AbstractReeModal
    void OnInitialize() override;
    StringW GetContent() override;

    static void OpenSongOrDownloadDialog(TrendingMapData mapDetail, UnityEngine::Transform* screenChild);
    static GlobalNamespace::BeatmapLevel* FetchMap(MapDetail mapDetail);
    static void OpenMap(GlobalNamespace::BeatmapLevel* map);
};

} 