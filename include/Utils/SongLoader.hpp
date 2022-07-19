#pragma once 
#include "beatsaber-hook/shared/utils/typedefs.h"

#include "custom-types/shared/macros.hpp" 

#include "songloader/shared/CustomTypes/SongLoaderBeatmapLevelPackCollectionSO.hpp" 
#include "songloader/shared/CustomTypes/SongLoaderCustomBeatmapLevelPack.hpp"

#include "GlobalNamespace/CustomPreviewBeatmapLevel.hpp" 
#include "GlobalNamespace/CustomBeatmapLevelCollection.hpp" 
#include "GlobalNamespace/CustomBeatmapLevelPack.hpp" 
#include "GlobalNamespace/StandardLevelInfoSaveData.hpp" 
#include "GlobalNamespace/EnvironmentInfoSO.hpp" 
#include "GlobalNamespace/BeatmapDataLoader.hpp" 
#include "UnityEngine/MonoBehaviour.hpp" 
#include "System/Collections/Generic/Dictionary_2.hpp"

#include <vector>

namespace RuntimeSongLoader {
    using DictionaryType = ::System::Collections::Generic::Dictionary_2<Il2CppString*, ::GlobalNamespace::CustomPreviewBeatmapLevel*>*;
}

DECLARE_CLASS_CODEGEN(RuntimeSongLoader, SongLoader, UnityEngine::MonoBehaviour, 
    private:
        static SongLoader* Instance;

        static std::vector<std::function<void(const std::vector<GlobalNamespace::CustomPreviewBeatmapLevel*>&)>> LoadedEvents;
        static std::mutex LoadedEventsMutex;

        static std::vector<std::function<void(SongLoaderBeatmapLevelPackCollectionSO*)>> RefreshLevelPacksEvents;
        static std::mutex RefreshLevelPacksEventsMutex;

        std::vector<GlobalNamespace::CustomPreviewBeatmapLevel*> LoadedLevels;

        void MenuLoaded();

        GlobalNamespace::StandardLevelInfoSaveData* GetStandardLevelInfoSaveData(const std::string& customLevelPath);
        GlobalNamespace::EnvironmentInfoSO* LoadEnvironmentInfo(Il2CppString* environmentName, bool allDirections);
        GlobalNamespace::CustomPreviewBeatmapLevel* LoadCustomPreviewBeatmapLevel(const std::string& customLevelPath, bool wip, GlobalNamespace::StandardLevelInfoSaveData* standardLevelInfoSaveData, std::string& outHash);
        
        void UpdateSongDuration(GlobalNamespace::CustomPreviewBeatmapLevel* level, const std::string& customLevelPath);
        float GetLengthFromMap(GlobalNamespace::CustomPreviewBeatmapLevel* level, const std::string& customLevelPath);

        List<GlobalNamespace::CustomPreviewBeatmapLevel*>* LoadSongsFromPath(std::string_view path, std::vector<std::string>& loadedPaths);

        DECLARE_INSTANCE_FIELD(DictionaryType, CustomLevels);
        DECLARE_INSTANCE_FIELD(DictionaryType, CustomWIPLevels);

        DECLARE_INSTANCE_FIELD(GlobalNamespace::BeatmapDataLoader*, beatmapDataLoader);

        DECLARE_INSTANCE_FIELD(SongLoaderCustomBeatmapLevelPack*, CustomLevelsPack);
        DECLARE_INSTANCE_FIELD(SongLoaderCustomBeatmapLevelPack*, CustomWIPLevelsPack);

        DECLARE_INSTANCE_FIELD(SongLoaderBeatmapLevelPackCollectionSO*, CustomBeatmapLevelPackCollectionSO);

        DECLARE_INSTANCE_FIELD(bool, IsLoading);
        DECLARE_INSTANCE_FIELD(bool, HasLoaded);

        DECLARE_INSTANCE_FIELD(bool, LoadingCancelled); //TODO: Implement this

        DECLARE_INSTANCE_FIELD(int, MaxFolders);
        DECLARE_INSTANCE_FIELD(int, CurrentFolder);

    public:
        static SongLoader* GetInstance();

        std::vector<GlobalNamespace::CustomPreviewBeatmapLevel*> GetLoadedLevels();

        static void AddSongsLoadedEvent(std::function<void(const std::vector<GlobalNamespace::CustomPreviewBeatmapLevel*>&)> event) {
            std::lock_guard<std::mutex> lock(LoadedEventsMutex);
            LoadedEvents.push_back(event);
        }

        static void AddRefreshLevelPacksEvent(std::function<void(SongLoaderBeatmapLevelPackCollectionSO*)> event) {
            std::lock_guard<std::mutex> lock(RefreshLevelPacksEventsMutex);
            RefreshLevelPacksEvents.push_back(event);
        }

        void RefreshLevelPacks();
        
        void RefreshSongs(bool fullRefresh, std::function<void(const std::vector<GlobalNamespace::CustomPreviewBeatmapLevel*>&)> songsLoaded = nullptr);

        void DeleteSong(std::string path, std::function<void()> finished);
        
        DECLARE_CTOR(ctor);
        DECLARE_SIMPLE_DTOR();

        DECLARE_INSTANCE_METHOD(void, Awake);
        DECLARE_INSTANCE_METHOD(void, Update);
)