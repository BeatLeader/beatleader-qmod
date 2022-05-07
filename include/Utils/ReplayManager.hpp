#pragma once

#include "include/Models/Replay.hpp"

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
   static void ProcessReplay(Replay const &replay, bool isOst, function<void(ReplayUploadStatus, string_view, float,
                                                                             int)> const &finished);
   static void RetryPosting(std::function<void(ReplayUploadStatus, std::string_view, float, int)> const& finished);

   static void TryPostReplay(string_view name, int tryIndex, function<void(ReplayUploadStatus, string_view, float,
                                                                           int)> const &finished);
   static string lastReplayFilename;
   static float GetTotalMultiplier(string_view modifiers);

   int GetLocalScore(string_view filename);
};
