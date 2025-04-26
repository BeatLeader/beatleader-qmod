#pragma once

#include "HMUI/ModalView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "Models/MapDetail.hpp"
#include "UI/Abstract/AbstractReeModal.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, MapDownloadDialogComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _loadingContainer);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, _finishedContainer);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, _finishedText);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _okButton);
};

namespace BeatLeader {

class MapDownloadDialog : public AbstractReeModal<MapDownloadDialogComponent*> {
public:
    using ComponentType = MapDownloadDialogComponent*;
    
    bool okButtonActive;
    StringW websiteUrl;
    bool mapDownloaded;
    MapDetail* context;

    void OnContextChanged() override;
    void OnResume() override;
    void SetContext(Il2CppObject* context) override {
        this->context = reinterpret_cast<MapDetail*>(context);
        OnContextChanged();
    }
    void HandleOkButtonClicked();

    // Override from AbstractReeModal
    void OnInitialize() override;
    StringW GetContent() override;

    static void OpenSongOrDownloadDialog(MapDetail mapDetail, UnityEngine::Transform* screenChild);
    static GlobalNamespace::BeatmapLevel* FetchMap(MapDetail mapDetail);
    static void OpenMap(GlobalNamespace::BeatmapLevel* map);
};

} 