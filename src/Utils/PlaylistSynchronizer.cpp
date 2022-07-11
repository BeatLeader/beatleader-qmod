#include "include/Utils/PlaylistSynchronizer.hpp"
#include "include/Utils/FileManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "Utils/StringUtils.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Models/Replay.hpp"

#include "main.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "songloader/shared/API.hpp"
// #include "playlistmanager/shared/PlaylistManager.hpp"

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "zip.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static bool mapsSynchronized = false;
static vector<string> mapsToDownload;

void done() {
    mapsSynchronized = true;
    WebUtils::GetAsync(WebUtils::API_URL + "user/oneclickdone", [](long httpCode, std::string data) {});
}

void DownloadBeatmap(string path, string hash, int index) {
    WebUtils::GetAsync(path, 64,
        [hash, index](long httpCode, std::string data) {
        auto targetFolder = RuntimeSongLoader::API::GetCustomLevelsPath() + hash;
        int args = 2;
        int statusCode = zip_stream_extract(data.data(), data.length(), targetFolder.c_str(), +[](const char* name, void* arg) -> int { return 0; }, &args);

        getLogger().info("%s", "Map downloaded");
        if (index + 1 < mapsToDownload.size()) {
            PlaylistSynchronizer::GetBeatmap(index + 1);
        } else {
            done();

            QuestUI::MainThreadScheduler::Schedule([] {
                getLogger().info("%s", "Refreshing songs");
                RuntimeSongLoader::API::RefreshSongs(false);
                RuntimeSongLoader::API::RefreshPacks(false);
            });
        }
    });
}

void PlaylistSynchronizer::GetBeatmap(int index) {
    string hash = mapsToDownload[index];
    getLogger().info("%s", ("Will download " + hash).c_str());
    WebUtils::GetJSONAsync("https://api.beatsaver.com/maps/hash/" + hash, [hash, index] (long status, bool error, rapidjson::Document const& result){
        if (status == 200) {
            DownloadBeatmap(result["versions"].GetArray()[0]["downloadURL"].GetString(), hash, index);
        } else if (index + 1 < mapsToDownload.size()) {
            GetBeatmap(index + 1);
        } else {
            done();
        }
    });
}

void ActuallySyncPlaylist() {
    if (PlayerController::currentPlayer == std::nullopt) return;

    WebUtils::GetAsync(WebUtils::API_URL + "user/oneclickplaylist", 64, [](long status, string result) {
        rapidjson::Document playlist;
        playlist.Parse(result.data());
        if (playlist.HasParseError() || !playlist.IsObject()) return;

        auto songs = playlist["songs"].GetArray();
        if ((int)songs.Size() == 0) return;

        ofstream outfile;
        outfile.open("sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/Playlists/BLSynced.json");
        outfile << result << endl;
        outfile.close();

        // ofstream outfile2;
        // outfile2.open("sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/PlaylistBackups/BLSynced.json");
        // outfile2 << result << endl;
        // outfile2.close();
        
        for (int index = 0; index < (int)songs.Size(); ++index)
        {
            auto const& song = songs[index];
            string hash = toLower(song["hash"].GetString());
            if (RuntimeSongLoader::API::GetLevelByHash(hash) == nullopt) {
                mapsToDownload.push_back(hash);
            }
        }

        if (mapsToDownload.size() == 0) {
            done();
        } else {
            PlaylistSynchronizer::GetBeatmap(0);
        }
    });
}

void PlaylistSynchronizer::SyncPlaylist() {
    RuntimeSongLoader::API::AddSongsLoadedEvent(
        [] (auto songs) {
            if (mapsSynchronized) return;
            ActuallySyncPlaylist();
        }
    );
    RuntimeSongLoader::API::AddRefreshLevelPacksEvent(
        [] (RuntimeSongLoader::SongLoaderBeatmapLevelPackCollectionSO* customBeatmapLevelPackCollectionSO) {
            // PlaylistManager::LoadPlaylists(customBeatmapLevelPackCollectionSO, true);
        }
    );
}
