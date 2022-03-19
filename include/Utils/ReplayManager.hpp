#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

enum ReplayUploadStatus {
    inProgress = 0,
    finished = 1,
    error = 2
};

class ReplayManager
{
private:
   static void TryPostReplay(string name, int tryIndex, std::function<void(ReplayUploadStatus, std::string, float)> finished);
   static string lastReplayFilename;
   static float GetTotalMultiplier(string modifiers);
public:
   static void ProcessReplay(Replay* replay, std::function<void(ReplayUploadStatus, std::string, float)> finished);
   static void RetryPosting(std::function<void(ReplayUploadStatus, std::string, float)> finished);
};
