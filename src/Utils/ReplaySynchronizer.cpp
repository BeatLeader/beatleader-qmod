#include "include/Utils/ReplaySynchronizer.hpp"
#include "include/Utils/FileManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/ReplayManager.hpp"
#include "include/API/PlayerController.hpp"
#include "include/Models/Replay.hpp"

#include "main.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filereadstream.h"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <filesystem>

ReplaySynchronizer::ReplaySynchronizer() noexcept
{
    if (PlayerController::currentPlayer == NULL) return;

    getLogger().info("Synchronizer init");

    ReplaySynchronizer* self = this;
    std::thread t (
            [self] {
        string directory = getDataDir(modInfo);
        filesystem::create_directories(directory);

        FILE *fp = fopen((directory + "sync.json").c_str(), "r");
        if (fp != NULL) {
            char buf[0XFFFF];
            FileReadStream input(fp, buf, sizeof(buf));
            self->statuses.ParseStream(input);
            fclose(fp);
        }

        if (!self->statuses.HasParseError() && self->statuses.IsObject()) {
            self->statuses.SetObject();
        }

        DIR *dir;
        string dirName = directory + "replays/";
        if ((dir = opendir(dirName.c_str())) != NULL) {
            self->Process(dir, dirName);
        }
    });
    t.detach();
}

void ReplaySynchronizer::updateStatus(string path, ReplayStatus status) {
    this->statuses.AddMember(rapidjson::StringRef(path), status, statuses.GetAllocator());
    Save();
}

void ReplaySynchronizer::Save() {
    string directory = getDataDir(modInfo);
    FILE* fp = fopen((directory + "sync.json").c_str(), "w");
 
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    
    Writer<FileWriteStream> writer(os);
    this->statuses.Accept(writer);
    
    fclose(fp);
}

void ReplaySynchronizer::Process(DIR *dir, string dirName) {
    struct dirent *ent;

    if ((ent = readdir(dir)) == NULL || PlayerController::currentPlayer == NULL) {
        closedir(dir);
        return;
    }

    string filename = ent->d_name;
    string filePath = dirName + filename;
    getLogger().info("Processing %s", filename.c_str());
    if (this->statuses.HasMember(filePath)) {
        if (ReplayStatus::topost == (ReplayStatus)this->statuses[filePath].GetInt()) {
            if (PlayerController::currentPlayer != NULL) {
                ReplaySynchronizer* self = this;
                ReplayManager::TryPostReplay(filePath, 0, [filePath, dir, dirName, self](bool finished, string description, float progress, int status) {
                    if (finished) {
                        if (status == 200) {
                            self->statuses[filePath].SetInt((int)ReplayStatus::uptodate);
                        } else if (status >= 400 && status < 500) {
                            self->statuses[filePath].SetInt((int)ReplayStatus::shouldnotpost);
                        }
                        self->Save();
                        self->Process(dir, dirName);
                    }
                });
            }
        } else {
            Process(dir, dirName);
        }
    } else {
        ReplayInfo* info = FileManager::ReadInfo(filePath);
        if (info == NULL || info->failTime > 0.001 || info->speed > 0.001 || (PlayerController::currentPlayer != NULL && info->playerID != PlayerController::currentPlayer->id)) {
            updateStatus(filePath, ReplayStatus::shouldnotpost);
            Process(dir, dirName);
        } else {
            string url = WebUtils::API_URL + "player/" + PlayerController::currentPlayer->id + "/scorevalue/" + info->hash + "/" + info->difficulty + "/" + info->mode;

            ReplaySynchronizer* self = this;
            WebUtils::GetJSONAsync(url, [info, self, dir, dirName, filePath](long status, bool error, rapidjson::Document& result){
                int score = result.GetInt();
                getLogger().info("Get score %i", score);
                if ((int)((float)info->score * ReplayManager::GetTotalMultiplier(info->modifiers)) > score) {
                    ReplayManager::TryPostReplay(filePath, 0, [filePath, dir, dirName, self](bool finished, string description, float progress, int status) {
                        if (finished) {
                            if (status == 200) {
                                self->updateStatus(filePath, ReplayStatus::uptodate);
                            } else if (status >= 400 && status < 500) {
                                self->updateStatus(filePath, ReplayStatus::shouldnotpost);
                            }
                            self->Process(dir, dirName);
                        }
                    });
                } else {
                    self->updateStatus(filePath, ReplayStatus::uptodate);
                    self->Process(dir, dirName);
                }
            });
        }
    }
}

