#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/constants.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/API/PlayerController.hpp"
#include "include/main.hpp"

#include <sys/stat.h>

string ReplayManager::lastReplayFilename = "";

void ReplayManager::ProcessReplay(Replay* replay, std::function<void(ReplayUploadStatus, std::string, float)> finished) {

    string filename = FileManager::ToFilePath(replay);

    struct stat buffer;
    if ((stat (filename.c_str(), &buffer) == 0)) {
        auto info = FileManager::ReadInfo(filename);
        getLogger().info("%s",("Modifiers " + info->modifiers + " " + replay->info->modifiers).c_str());
        if (info != NULL &&
        (float)info->score * GetTotalMultiplier(info->modifiers) >= (float)replay->info->score * GetTotalMultiplier(replay->info->modifiers)) {
            finished(ReplayUploadStatus::finished, "<color=#008000ff>Score not beaten</color>", 0);
            return; 
        }
    }
    FileManager::WriteReplay(replay);

    getLogger().info("%s",("Replay saved " + filename).c_str());
    
    if (replay->info->failTime > 0.001 || replay->info->speed > 0.001) { 
        finished(ReplayUploadStatus::finished, "<color=#008000ff>Failed attempt was saved!</color>", 0);
        return; 
    }
    if(PlayerController::currentPlayer != NULL)
        TryPostReplay(filename, 0, finished);
}

void ReplayManager::TryPostReplay(string name, int tryIndex, std::function<void(ReplayUploadStatus, std::string, float)> finished) {
    struct stat file_info;
    lastReplayFilename = name;
    stat(name.c_str(), &file_info);
    if (tryIndex == 0) {
        getLogger().info("%s",("Started posting " + to_string(file_info.st_size)).c_str());
        finished(ReplayUploadStatus::inProgress, "<color=#b103fcff>Posting replay...", 0);
    }
    
    WebUtils::PostFileAsync(API_URL + "replayoculus", fopen(name.c_str(), "rb"), (long)file_info.st_size, 100, [name, tryIndex, finished](long statusCode, string result) {
        if (statusCode != 200 && tryIndex < 2) {
            getLogger().info("%s", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex) + " " + result).c_str());
            if (statusCode == 100) {
                result = "Timed out";
            }
            finished(ReplayUploadStatus::inProgress, "<color=#ffff00ff>Retrying posting replay after " + to_string(statusCode) + " try #" + to_string(tryIndex) + " " + result + "</color>", 0);
            TryPostReplay(name, tryIndex + 1, finished);
        } else if (statusCode == 200) {
            getLogger().info("Replay was posted!");
            finished(ReplayUploadStatus::finished, "<color=#008000ff>Replay was posted!</color>", 100);
        } else {
            getLogger().error("Replay was not posted!");
            if (statusCode == 100) {
                result = "Timed out";
            }
            finished(ReplayUploadStatus::error, "<color=#ff0000ff>Replay was not posted! Error: " + result, 0);
        }
    }, [finished](float percent) {
        finished(ReplayUploadStatus::inProgress, "<color=#b103fcff>Posting replay: " + to_string_wprecision(percent, 2) + "%", percent);
    });
}

void ReplayManager::RetryPosting(std::function<void(ReplayUploadStatus, std::string, float)> finished) {
    TryPostReplay(lastReplayFilename, 0, finished);
};

float ReplayManager::GetTotalMultiplier(string modifiers) {
    float multiplier = 1;

    auto modifierValues = ModifiersManager::modifiers;
    for (std::map<string, float>::iterator iter = modifierValues.begin(); iter != modifierValues.end(); ++iter)
    {   
        if (modifiers.find(iter->first) != string::npos) { multiplier += modifierValues[iter->first]; }
    }

    return multiplier;
}