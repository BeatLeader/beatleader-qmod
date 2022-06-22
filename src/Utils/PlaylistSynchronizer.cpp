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

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "zip.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static bool mapsSynchronized = false;
static int songsToDownload = 0;

void DownloadBeatmap(string path, string hash) {
    WebUtils::GetAsync(path, 64,
        [hash](long httpCode, std::string data) {
        auto targetFolder = RuntimeSongLoader::API::GetCustomLevelsPath() + hash;
        int args = 2;
        int statusCode = zip_stream_extract(data.data(), data.length(), targetFolder.c_str(), +[](const char* name, void* arg) -> int { return 0; }, &args);

        songsToDownload--;
        getLogger().info("%s", "Map downloaded");
        if (songsToDownload == 0) {
            mapsSynchronized = true;

            WebUtils::GetAsync(WebUtils::API_URL + "user/oneclickdone", [](long httpCode, std::string data) {});

            QuestUI::MainThreadScheduler::Schedule([] {
                getLogger().info("%s", "Refreshing songs");
                RuntimeSongLoader::API::RefreshSongs(false);
                // RuntimeSongLoader::API::RefreshPacks(false);
            });
        }
    });
}

void GetBeatmap(std::string hash) {
    WebUtils::GetJSONAsync("https://api.beatsaver.com/maps/hash/" + hash, [hash] (long status, bool error, rapidjson::Document const& result){
        if (status == 200) {
            songsToDownload++;
            DownloadBeatmap(result["versions"].GetArray()[0]["downloadURL"].GetString(), hash);
        }
    });
}

void ActuallySyncPlaylist() {
    if (PlayerController::currentPlayer == std::nullopt) return;

    WebUtils::GetAsync("https://cdn.beatleader.xyz/playlists/" + PlayerController::currentPlayer->id + "oneclick.bplist", 64, [](long status, string result) {
        string playlistPath = "sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/Playlists/BLSynced.json";
        
        rapidjson::Document playlist;
        playlist.Parse(result.data());
        if (playlist.HasParseError() || !playlist.IsObject()) return;

        auto songs = playlist["songs"].GetArray();
        if ((int)songs.Size() == 0) return;

        ofstream outfile;
        outfile.open(playlistPath);
        outfile << result << endl;
        outfile.close();
        
        for (int index = 0; index < (int)songs.Size(); ++index)
        {
            auto const& song = songs[index];
            string hash = toLower(song["hash"].GetString());
            if (RuntimeSongLoader::API::GetLevelByHash(hash) == nullopt) {
                getLogger().info("%s", ("Will download " + hash).c_str());
                GetBeatmap(hash);
            }
        }
    });
}

void PlaylistSynchronizer::SyncPlaylist()
{
    RuntimeSongLoader::API::AddSongsLoadedEvent(
        [] (auto songs) {
            if (mapsSynchronized) return;
            ActuallySyncPlaylist();
        }
    );
}
