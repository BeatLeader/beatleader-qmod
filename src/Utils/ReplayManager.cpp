#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/API/PlayerController.hpp"
#include "include/main.hpp"

#include <sys/stat.h>
#include <stdio.h>

string ReplayManager::lastReplayFilename = "";

void ReplayManager::ProcessReplay(Replay const &replay, bool isOst, function<void(ReplayUploadStatus, string, float,
                                                                                  int)> const &finished) {

    if(!UploadEnabled()) {
        finished(ReplayUploadStatus::finished, "<color=#008000ff>Upload disabled</color>", 0, -1);
        return;
    }
    
    string filename = FileManager::ToFilePath(replay);
    lastReplayFilename = filename;
    
    FileManager::WriteReplay(replay);

    getLogger().info("%s",("Replay saved " + filename).c_str());
    
    if (replay.info.failTime > 0.001 || replay.info.speed > 0.001) {
        finished(ReplayUploadStatus::finished, "<color=#008000ff>Failed attempt was saved!</color>", 0, -1);
        return; 
    }
    if(isOst)
        return;
    if(PlayerController::currentPlayer != std::nullopt)
        TryPostReplay(filename, 0, finished);
}

void ReplayManager::TryPostReplay(string name, int tryIndex, function<void(ReplayUploadStatus, string, float,
                                                                                int)> const &finished) {
    struct stat file_info;
    stat(name.data(), &file_info);
    if (tryIndex == 0) {
        getLogger().info("%s",("Started posting " + to_string(file_info.st_size)).c_str());
        finished(ReplayUploadStatus::inProgress, "<color=#b103fcff>Posting replay...", 0, 0);
    }
    FILE *replayFile = fopen(name.data(), "rb");
    
    WebUtils::PostFileAsync(WebUtils::API_URL + "replayoculus", replayFile, (long)file_info.st_size, 100, [name, tryIndex, finished, replayFile](long statusCode, string result) {
        fclose(replayFile);
        if (statusCode != 200 && tryIndex < 2) {
            getLogger().info("%s", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex) + " " + std::string(result)).c_str());
            if (statusCode == 100) {
                result = "Timed out";
            }
            finished(ReplayUploadStatus::inProgress, "<color=#ffff00ff>Retrying posting replay after " + to_string(statusCode) + " try #" + to_string(tryIndex) + " " + std::string(result) + "</color>", 0, statusCode);
            TryPostReplay(name, tryIndex + 1, finished);
        } else if (statusCode == 200) {
            getLogger().info("Replay was posted!");
            finished(ReplayUploadStatus::finished, "<color=#008000ff>Replay was posted!</color>", 100, statusCode);
            if (!getModConfig().Save.GetValue()) {
                remove(name.data());
            }
        } else {
            getLogger().error("Replay was not posted!");
            if (statusCode == 100) {
                result = "Timed out";
            }
            finished(ReplayUploadStatus::error, std::string("<color=#ff0000ff>Replay was not posted! Error: " + result), 0, statusCode);
        }
    }, [finished](float percent) {
        finished(ReplayUploadStatus::inProgress, "<color=#b103fcff>Posting replay: " + to_string_wprecision(percent, 2) + "%", percent, 0);
    });
}

void ReplayManager::RetryPosting(const std::function<void(ReplayUploadStatus, std::string, float, int)>& finished) {
    TryPostReplay(lastReplayFilename, 0, finished);
};

int ReplayManager::GetLocalScore(string filename) {
    struct stat buffer;
    if ((stat (filename.data(), &buffer) == 0)) {
        auto info = FileManager::ReadInfo(filename);
        if (info != std::nullopt) {
            return (int)((float)info->score * GetTotalMultiplier(info->modifiers)); 
        }
    }

    return 0;
} 

float ReplayManager::GetTotalMultiplier(string modifiers) {
    float multiplier = 1;

    auto modifierValues = ModifiersManager::modifiers;
    for (auto iter = modifierValues.begin(); iter != modifierValues.end(); ++iter)
    {   
        if (modifiers.find(iter->first) != string::npos) { multiplier += modifierValues[iter->first]; }
    }

    return multiplier;
}