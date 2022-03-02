#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include <sys/stat.h>

void ReplayManager::ProcessReplay(Replay* replay) {
    string filename = FileManager::WriteReplay(replay);

    if (replay->info->failTime < 0.001 && replay->info->speed < 0.001) { return; }
    TryPostReplay(filename, 0);
}

void ReplayManager::TryPostReplay(string name, int tryIndex) {
    struct stat file_info;
    stat(name.c_str(), &file_info);
    getLogger().error("%s",("BLYAT start posting " + to_string(file_info.st_size)).c_str());

    WebUtils::PostFileAsync("https://beatleader.azurewebsites.net/replayoculus", fopen(name.c_str(), "rb"), (long)file_info.st_size, 50, [name, tryIndex](long statusCode, string result) {
        if (statusCode != 200 && tryIndex < 2) {
            getLogger().error("%s", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex)).c_str());
            TryPostReplay(name, tryIndex + 1);
        } else if (statusCode == 200) {
            getLogger().info("Replay was posted");
        } else {
            getLogger().error("Replay was not posted");
        }
    });
}