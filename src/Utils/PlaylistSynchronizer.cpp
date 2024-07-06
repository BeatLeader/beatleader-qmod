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

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "songcore/shared/SongCore.hpp"

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
        auto targetFolder = string(SongCore::API::Loading::GetPreferredCustomLevelPath()) + "/" + hash;
        int args = 2;
        int statusCode = zip_stream_extract(data.data(), data.length(), targetFolder.c_str(), +[](const char* name, void* arg) -> int { return 0; }, &args);

        BeatLeaderLogger.info("{}", "Map downloaded");
        if (index + 1 < mapsToDownload.size()) {
            PlaylistSynchronizer::GetBeatmap(index + 1);
        } else {
            done();

            BSML::MainThreadScheduler::Schedule([] {
                BeatLeaderLogger.info("{}", "Refreshing songs");
                SongCore::API::Loading::RefreshSongs(false);
                SongCore::API::Loading::RefreshLevelPacks();
            });
        }
    });
}

void PlaylistSynchronizer::GetBeatmap(int index) {
    string hash = mapsToDownload[index];
    BeatLeaderLogger.info("{}", ("Will download " + hash).c_str());
    WebUtils::GetJSONAsync("https://api.beatsaver.com/maps/hash/" + hash, [hash, index] (long status, bool error, rapidjson::Document const& result){
        if (status == 200 && !error && result.HasMember("versions")) {
            DownloadBeatmap(result["versions"].GetArray()[0]["downloadURL"].GetString(), hash, index);
        } else if (index + 1 < mapsToDownload.size()) {
            GetBeatmap(index + 1);
        } else {
            done();
        }
    });
}

void DownloadPlaylist(
    string url, 
    string name,
    bool checkSize,
    function<void(rapidjson::GenericArray<false, rapidjson::Value>)> const &completion) {

    WebUtils::GetAsync(url, 64, [completion, checkSize, name](long status, string result) {
        rapidjson::Document playlist;
        playlist.Parse(result.data());
        if (playlist.HasParseError() || !playlist.IsObject() || !playlist.HasMember("songs")) return;

        auto songs = playlist["songs"].GetArray();
        if (checkSize && (int)songs.Size() == 0) return;

        ofstream outfile;
        outfile.open("sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/Playlists/" + name + ".bplist_BMBF.json");
        outfile << result << endl;
        outfile.close();

        ofstream outfile2;
        outfile2.open("sdcard/ModData/com.beatgames.beatsaber/Mods/PlaylistManager/PlaylistBackups/" + name + ".bplist_BMBF.json");
        outfile2 << result << endl;
        outfile2.close();

        completion(songs);
    });
}

void ActuallySyncPlaylist() {
    if (PlayerController::currentPlayer == std::nullopt) return;

    auto parts = split(PlayerController::currentPlayer->playlistsToInstall, ",");

    for (string playlist : parts) {
        DownloadPlaylist(WebUtils::API_URL + "playlist/" + playlist, playlist, true, [](auto songs) {
            BSML::MainThreadScheduler::Schedule([] {
                SongCore::API::Loading::RefreshLevelPacks();
            });
        });
    }

    WebUtils::RequestAsync(WebUtils::API_URL + "user/playlist/toInstall", "DELETE", 200, [](long httpCode, std::string data) {});

    DownloadPlaylist(WebUtils::API_URL + "user/oneclickplaylist", "BLSynced", true, [](auto songs) {
        for (int index = 0; index < (int)songs.Size(); ++index)
        {
            auto const& song = songs[index];
            string hash = toLower(song["hash"].GetString());
            if (!SongCore::API::Loading::GetLevelByHash(hash)) {
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
    SongCore::API::Loading::GetSongsLoadedEvent() +=
        [] (auto songs) {
            if (mapsSynchronized) return;
            ActuallySyncPlaylist();
        };
}

void PlaylistSynchronizer::InstallPlaylist(string url, string filename) {
    DownloadPlaylist(url, filename, true, [](auto songs) {
        BSML::MainThreadScheduler::Schedule([] {
            SongCore::API::Loading::RefreshSongs(false);
            SongCore::API::Loading::RefreshLevelPacks();
        });
    });
}
