#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/API/PlayerController.hpp"
#include "include/main.hpp"

#include <sys/stat.h>
#include <stdio.h>

string ReplayManager::lastReplayFilename = "";
PlayEndData ReplayManager::lastReplayStatus = PlayEndData(LevelEndType::Fail, 0);

void ReplayManager::ProcessReplay(Replay const &replay, PlayEndData status, bool skipUpload, function<void(ReplayUploadStatus, std::optional<string>, string, float,
                                                                                  int)> const &finished) {
    if (status.GetEndType() == LevelEndType::Unknown) {
        return;
    }
    
    string filename = FileManager::ToFilePath(replay);
    lastReplayFilename = filename;

    FileManager::WriteReplay(replay);
    BeatLeaderLogger.info("{}",("Replay saved " + filename).c_str());

    if(!UploadEnabled()) {
        finished(ReplayUploadStatus::finished, std::nullopt, "<color=#BB2020ff>Upload disabled. But replay was saved.</color>", 0, -1);
        return;
    }    
    
    if (replay.info.failTime > 0.001 || replay.info.speed > 0.001) {
        finished(ReplayUploadStatus::finished, std::nullopt, "<color=#BB2020ff>Failed attempt was saved!</color>", 0, -1);
    }
    if(skipUpload)
        return;
    if(PlayerController::currentPlayer != std::nullopt)
        TryPostReplay(filename, status, 0, finished);
}

void ReplayManager::TryPostReplay(string name, PlayEndData status, int tryIndex, function<void(ReplayUploadStatus, std::optional<string>, string, float,
                                                                                int)> const &finished) {
    struct stat file_info;
    stat(name.data(), &file_info);

    lastReplayStatus = status;
    bool runCallback = status.GetEndType() == LevelEndType::Clear;
    if (tryIndex == 0) {
        BeatLeaderLogger.info("{}",("Started posting " + to_string(file_info.st_size)).c_str());
        if (runCallback) {
            finished(ReplayUploadStatus::inProgress, std::nullopt, "<color=#b103fcff>Posting replay...", 0, 0);
        }
    }
    FILE *replayFile = fopen(name.data(), "rb");
    chrono::steady_clock::time_point replayPostStart = chrono::steady_clock::now();
    
    WebUtils::PostFileAsync(WebUtils::API_URL + "replayoculus" + status.ToQueryString(), replayFile, (long)file_info.st_size, [name, tryIndex, finished, replayFile, replayPostStart, runCallback, status](long statusCode, string result, string headers) {
        fclose(replayFile);
        if ((statusCode >= 450 || statusCode < 200) && tryIndex < 2) {
            BeatLeaderLogger.info("{}", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex) + " " + std::string(result)).c_str());
            if (statusCode == 100) {
                result = "Timed out";
            }
            if (runCallback) {
                finished(ReplayUploadStatus::inProgress, std::nullopt, "<color=#ffff00ff>Retrying posting replay after " + to_string(statusCode) + " try #" + to_string(tryIndex) + " " + std::string(result) + "</color>", 0, statusCode);
            }
            TryPostReplay(name, status, tryIndex + 1, finished);
        } else if (statusCode == 200) {
            auto duration = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - replayPostStart).count();
            BeatLeaderLogger.info("{}", ("Replay was posted! It took: " + to_string((int)duration) + "msec. \n").c_str());
            if (runCallback) {
                finished(ReplayUploadStatus::finished, result, "<color=#20BB20ff>Replay was posted!</color>", 100, statusCode);
            }
            if (!getModConfig().SaveLocalReplays.GetValue()) {
                remove(name.data());
            }
        } else {
            if (statusCode == 100 || statusCode == 0) {
                statusCode = 100;
                result = "Timed out";
            }
            BeatLeaderLogger.error("{}", ("Replay was not posted! " + to_string(statusCode) + result).c_str());
            if (runCallback) {
                finished(ReplayUploadStatus::error, std::nullopt, std::string("<color=#BB2020ff>Replay was not posted. " + result), 0, statusCode);
            }
        }
    }, [finished, runCallback](float percent) {
        if (runCallback) {
            finished(ReplayUploadStatus::inProgress, std::nullopt, "<color=#b103fcff>Posting replay: " + to_string_wprecision(percent, 2) + "%", percent, 0);
        }
    });
}

void ReplayManager::RetryPosting(const std::function<void(ReplayUploadStatus, std::optional<string>, string, float, int)>& finished) {
    TryPostReplay(lastReplayFilename, lastReplayStatus, 0, finished);
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

    auto modifierValues = ModifiersUI::songModifiers;
    if(!modifierValues.empty()) {
        for (auto& [key, value] : modifierValues){
            if (modifiers.find(key) != string::npos) {
                multiplier += value;
            }
        }
    }

    return multiplier;
}