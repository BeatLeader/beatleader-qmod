#pragma once

#include "shared/Models/Replay.hpp"
#include "include/Core/ReplayRecorder.hpp"

using namespace std;

#include <string>

enum struct ReplayUploadStatus {
    inProgress = 0,
    finished = 1,
    error = 2
};

class ReplayManager
{  
public:
   static void ProcessReplay(Replay const &replay, PlayEndData status, bool skipUpload, function<void(ReplayUploadStatus, string, float,
                                                                             int)> const &finished);
   static void RetryPosting(function<void(ReplayUploadStatus, string, float, int)> const& finished);

   static void TryPostReplay(string name, PlayEndData status, int tryIndex, function<void(ReplayUploadStatus, string, float,
                                                                           int)> const &finished);
   static string lastReplayFilename;
   static PlayEndData lastReplayStatus;
   static float GetTotalMultiplier(string modifiers);

   int GetLocalScore(string filename);
};
