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
        char buf[0XFFFF];
        FileReadStream input(fp, buf, sizeof(buf));
        self->statuses.ParseStream(input);
        fclose(fp);

        if (!self->statuses.HasParseError() || !self->statuses.IsObject()) {
            self->statuses.SetObject();
        }

        DIR *dir;
        
        if ((dir = opendir((directory + "\\replays").c_str())) != NULL) {
            self->Process(dir);
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

void ReplaySynchronizer::Process(DIR *dir) {
    struct dirent *ent;

    if ((ent = readdir(dir)) == NULL || PlayerController::currentPlayer == NULL) {
        closedir(dir);
        return;
    }

    string filename = ent->d_name;
    getLogger().info("Processing %s", filename.c_str());
    if (this->statuses.HasMember(filename)) {
        if (ReplayStatus::topost == (ReplayStatus)this->statuses[filename].GetInt()) {
            if (PlayerController::currentPlayer != NULL) {
                ReplaySynchronizer* self = this;
                ReplayManager::TryPostReplay(filename, 0, [filename, dir, self](bool finished, string description, float progress, int status) {
                    if (finished) {
                        if (status == 200) {
                            self->statuses[filename].SetInt((int)ReplayStatus::uptodate);
                        } else if (status >= 400 && status < 500) {
                            self->statuses[filename].SetInt((int)ReplayStatus::shouldnotpost);
                        }
                        self->Save();
                        self->Process(dir);
                    }
                });
            }
        }
    } else {
        ReplayInfo* info = FileManager::ReadInfo(filename);
        if (info == NULL || info->failTime > 0.001 || info->speed > 0.001) {
            this->statuses[filename].SetInt((int)ReplayStatus::shouldnotpost);
            Save();
            Process(dir);
        } else if (PlayerController::currentPlayer != NULL) {
            string url = WebUtils::API_URL + "player/" + PlayerController::currentPlayer->id + "/scorevalue/" + info->hash + "/" + info->difficulty + "/" + info->mode;

            ReplaySynchronizer* self = this;
            WebUtils::GetJSONAsync(url, [info, self, dir, filename](long status, bool error, rapidjson::Document& result){
                int score = result.GetInt();
                if ((int)((float)info->score * ReplayManager::GetTotalMultiplier(info->modifiers)) > score) {
                    ReplayManager::TryPostReplay(filename, 0, [filename, dir, self](bool finished, string description, float progress, int status) {
                        if (finished) {
                            if (status == 200) {
                                self->statuses[filename].SetInt((int)ReplayStatus::uptodate);
                            } else if (status >= 400 && status < 500) {
                                self->statuses[filename].SetInt((int)ReplayStatus::shouldnotpost);
                            }
                            self->Save();
                            self->Process(dir);
                        }
                    });
                } else {
                    self->statuses[filename].SetInt((int)ReplayStatus::uptodate);
                    self->Save();
                    self->Process(dir);
                }
            });
        }
    }
}

