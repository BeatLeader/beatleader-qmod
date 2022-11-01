#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

class FileManager
{
private:
    /* data */
public:
   static void EnsureReplaysFolderExists();
   static void WriteReplay(Replay const &replay);
   static optional<ReplayInfo> ReadInfo(string replayPath);
   static string ToFilePath(Replay const &replay);
};
