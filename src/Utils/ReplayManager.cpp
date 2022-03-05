#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include <sys/stat.h>

string ReplayManager::lastReplayFilename = "";

void ReplayManager::ProcessReplay(Replay* replay, std::function<void(ReplayUploadStatus, std::string)> finished) {
    string filename = FileManager::WriteReplay(replay);

    getLogger().info("%s",("Replay saved " + filename).c_str());
    
    if (replay->info->failTime > 0.001 || replay->info->speed > 0.001) { 
        finished(ReplayUploadStatus::finished, "<color=#008000ff>Failed attempt was saved!</color>");
        return; 
    }
    finished(ReplayUploadStatus::inProgress, "<color=#ffff00ff>Replays was saved!</color>");
    TryPostReplay(filename, 0, finished);
}

void ReplayManager::TryPostReplay(string name, int tryIndex, std::function<void(ReplayUploadStatus, std::string)> finished) {
    struct stat file_info;
    lastReplayFilename = name;
    stat(name.c_str(), &file_info);
    getLogger().info("%s",("Started posting " + to_string(file_info.st_size)).c_str());

    WebUtils::PostFileAsync("https://beatleader.azurewebsites.net/replayoculus", fopen(name.c_str(), "rb"), (long)file_info.st_size, 50, [name, tryIndex, finished](long statusCode, string result) {
        if (statusCode != 200 && tryIndex < 2) {
            getLogger().info("%s", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex) + " " + result).c_str());
            finished(ReplayUploadStatus::inProgress, "<color=#ffff00ff>Retrying posting replay after " + to_string(statusCode) + " try" + to_string(tryIndex) + " " + result + "</color>");
            TryPostReplay(name, tryIndex + 1, finished);
        } else if (statusCode == 200) {
            getLogger().info("Replay was posted!");
            finished(ReplayUploadStatus::finished, "<color=#008000ff>Replay was posted!</color>");
        } else {
            getLogger().error("Replay was not posted!");
            finished(ReplayUploadStatus::error, "<color=#ff0000ff>Replay was not posted! Error code: " + to_string(statusCode));
        }
    });
}

void ReplayManager::RetryPosting(std::function<void(ReplayUploadStatus, std::string)> finished) {
    TryPostReplay(lastReplayFilename, 0, finished);
};