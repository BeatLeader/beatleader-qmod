#include "include/Utils/FileManager.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "include/API/PlayerController.hpp"
#include "shared/Models/ScoreUpload.hpp"
#include "shared/Models/Score.hpp"
#include "include/main.hpp"

#include <filesystem>
#include <sys/stat.h>
#include <stdio.h>

namespace {
    bool lastReplayShouldPersist = false;

    bool ShouldKeepReplayLocally(PlayEndData const& status) {
        if (!getModConfig().SaveLocalReplays.GetValue()) {
            return false;
        }

        switch (status.GetEndType()) {
            case LevelEndType::Fail:
                return getModConfig().KeepFailReplays.GetValue();
            case LevelEndType::Quit:
            case LevelEndType::Restart:
                return getModConfig().KeepExitReplays.GetValue();
            case LevelEndType::Practice:
                return getModConfig().KeepPracticeReplays.GetValue();
            case LevelEndType::Clear:
                return true;
            default:
                return false;
        }
    }

    void DeleteMatchingReplays(Replay const& replay) {
        std::error_code errorCode;
        auto replaysDirectory = std::filesystem::path(getDataDir(modInfo)) / "replays";
        if (!std::filesystem::exists(replaysDirectory, errorCode)) {
            return;
        }

        for (auto const& entry : std::filesystem::directory_iterator(replaysDirectory, errorCode)) {
            if (errorCode || !entry.is_regular_file() || entry.path().extension() != ".bsor") {
                continue;
            }

            auto replayInfo = FileManager::ReadInfo(entry.path().string());
            if (!replayInfo.has_value()) {
                continue;
            }

            if (replayInfo->playerID != replay.info.playerID
                || replayInfo->songName != replay.info.songName
                || replayInfo->difficulty != replay.info.difficulty
                || replayInfo->mode != replay.info.mode
                || replayInfo->hash != replay.info.hash) {
                continue;
            }

            remove(entry.path().string().c_str());
        }
    }
}

string ReplayManager::lastReplayFilename = "";
PlayEndData ReplayManager::lastReplayStatus = PlayEndData(LevelEndType::Fail, 0);

void ReplayManager::ProcessReplay(Replay const &replay, PlayEndData status, bool skipUpload, function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float,
                                                                                  int)> const &finished) {
    if (status.GetEndType() == LevelEndType::Unknown) {
        return;
    }

    auto keepLocalReplay = ShouldKeepReplayLocally(status);
    auto shouldUpload = !skipUpload && PlayerController::currentPlayer != std::nullopt && UploadEnabled();

    lastReplayFilename.clear();
    lastReplayStatus = status;
    lastReplayShouldPersist = keepLocalReplay;

    if (!keepLocalReplay && !shouldUpload) {
        if (!UploadEnabled()) {
            finished(
                ReplayUploadStatus::finished,
                std::nullopt,
                "<color=#BB2020ff>Upload disabled. Replay was not kept locally.</color>",
                0,
                -1
            );
        }
        return;
    }

    if (keepLocalReplay && !getModConfig().SaveEveryReplayAttempt.GetValue()) {
        DeleteMatchingReplays(replay);
    }

    string filename = FileManager::ToFilePath(replay, status);
    lastReplayFilename = filename;

    FileManager::WriteReplay(filename, replay);
    BeatLeaderLogger.info("{}",("Replay saved " + filename).c_str());

    if(!UploadEnabled()) {
        finished(
            ReplayUploadStatus::finished,
            std::nullopt,
            keepLocalReplay
                ? "<color=#BB2020ff>Upload disabled. But replay was saved.</color>"
                : "<color=#BB2020ff>Upload disabled. Replay was not kept locally.</color>",
            0,
            -1
        );
        return;
    }    
    
    if(skipUpload)
        return;
    if(PlayerController::currentPlayer != std::nullopt)
        TryPostReplay(filename, status, 0, keepLocalReplay, finished);
}

void ReplayManager::TryPostReplay(string name, PlayEndData status, int tryIndex, bool keepLocalReplay, function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float,
                                                                                int)> const &finished) {
    struct stat file_info;
    if (stat(name.data(), &file_info) != 0) {
        return;
    }

    lastReplayStatus = status;
    lastReplayShouldPersist = keepLocalReplay;
    bool runCallback = status.GetEndType() == LevelEndType::Clear;
    if (tryIndex == 0) {
        BeatLeaderLogger.info("{}",("Started posting " + to_string(file_info.st_size)).c_str());
        if (runCallback) {
            finished(ReplayUploadStatus::inProgress, std::nullopt, "<color=#b103fcff>Posting replay...", 0, 0);
        }
    }
    FILE *replayFile = fopen(name.data(), "rb");
    chrono::steady_clock::time_point replayPostStart = chrono::steady_clock::now();
    
    WebUtils::PostFileAsync(WebUtils::API_URL + "v2/replayoculus" + status.ToQueryString(), replayFile, (long)file_info.st_size, [name, tryIndex, finished, replayFile, replayPostStart, runCallback, status, keepLocalReplay](long statusCode, string result, string headers) {
        fclose(replayFile);
        if ((statusCode >= 450 || statusCode < 200) && tryIndex < 2) {
            BeatLeaderLogger.info("{}", ("Retrying posting replay after " + to_string(statusCode) + " #" + to_string(tryIndex) + " " + std::string(result)).c_str());
            if (statusCode == 100) {
                result = "Timed out";
            }
            if (runCallback) {
                finished(ReplayUploadStatus::inProgress, std::nullopt, "<color=#ffff00ff>Retrying posting replay after " + to_string(statusCode) + " try #" + to_string(tryIndex) + " " + std::string(result) + "</color>", 0, statusCode);
            }
            TryPostReplay(name, status, tryIndex + 1, keepLocalReplay, finished);
        } else if (statusCode == 200) {
            auto duration = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - replayPostStart).count();
            BeatLeaderLogger.info("{}", ("Replay was posted! It took: " + to_string((int)duration) + "msec. \n").c_str());
            rapidjson::Document document;
            document.Parse(result.data());
            ScoreUpload object = ScoreUpload(document.GetObject());
            switch (object.status) {
                case ScoreUploadStatus::Uploaded:
                    finished(ReplayUploadStatus::finished, object, "<color=#20BB20ff>Replay was posted!</color>", 100, statusCode);
                    break;
                case ScoreUploadStatus::Attempt:
                    finished(ReplayUploadStatus::finished, object, "<color=#BB2020ff>Failed attempt was saved!</color>", 0, -1);
                    break;
                case ScoreUploadStatus::NonPB:
                    finished(ReplayUploadStatus::finished, object, "<color=#BB2020ff>Attempt was saved, but it was worse than your existing score!</color>", 0, -1);
                    break;
                case ScoreUploadStatus::Error:
                    finished(ReplayUploadStatus::error, object, "<color=#BB2020ff>Replay was not posted. " + object.description + "</color>", 0, statusCode);
                    break;
            }
            if (!keepLocalReplay) {
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

void ReplayManager::RetryPosting(const std::function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float, int)>& finished) {
    if (lastReplayFilename.empty()) {
        return;
    }

    TryPostReplay(lastReplayFilename, lastReplayStatus, 0, lastReplayShouldPersist, finished);
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
