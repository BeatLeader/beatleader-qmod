#pragma once

#include "shared/Models/Replay.hpp"
#include "include/Core/ReplayRecorder.hpp"
#include "shared/Models/ScoreUpload.hpp"

using namespace std;

#include <string>

enum struct ReplayUploadStatus {
    inProgress = 0,
    finished = 1,
    error = 2
};

struct PlayUpload {
    int play = 0;
    bool started = false;
    bool done = false;
    std::optional<ScoreUpload> result;
};

class ReplayManager
{  
public:
   static PlayUpload const& LastPlayUpload();
   static void AddPlayUploadListener(function<void()> listener);

   static void ProcessReplay(Replay const &replay, PlayEndData status, bool skipUpload, function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float,
                                                                             int)> const &finished);
   static void RetryPosting(function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float, int)> const& finished);

   static void TryPostReplay(string name, PlayEndData status, int tryIndex, bool keepLocalReplay, function<void(ReplayUploadStatus, std::optional<ScoreUpload>, string, float,
                                                                           int)> const &finished, int play);
   static string lastReplayFilename;
   static PlayEndData lastReplayStatus;
   static float GetTotalMultiplier(string modifiers);

   static int GetLocalScore(string filename);
};
