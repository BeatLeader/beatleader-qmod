#pragma once

#include "include/Core/ReplayRecorder.hpp"
#include "shared/Models/Replay.hpp"

using namespace std;

#include <string>

class FileManager
{
private:
    /* data */
public:
   static void EnsureReplaysFolderExists();
   static void WriteReplay(string const& replayPath, Replay const &replay);
   static optional<ReplayInfo> ReadInfo(string replayPath);
   static string ToFilePath(Replay const &replay, PlayEndData const& playEndData);
};
