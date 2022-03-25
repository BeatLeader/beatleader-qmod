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
public:
   static void ProcessReplay(Replay* replay, bool isOst, std::function<void(ReplayUploadStatus, std::string, float, int)> finished);
   static void RetryPosting(std::function<void(ReplayUploadStatus, std::string, float, int)> finished);

   static void TryPostReplay(string name, int tryIndex, std::function<void(ReplayUploadStatus, std::string, float, int)> finished);
   static string lastReplayFilename;
   static float GetTotalMultiplier(string modifiers);

   int GetLocalScore(string filename);
};
