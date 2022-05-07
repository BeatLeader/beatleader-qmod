#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

class FileManager
{
private:
    /* data */
public:
   static void WriteReplay(Replay const &replay);
   static std::optional<ReplayInfo> ReadInfo(string_view replayPath);
   static string ToFilePath(Replay const &replay);
};
