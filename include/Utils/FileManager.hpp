#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

class FileManager
{
private:
    /* data */
public:
   static void WriteReplay(Replay* replay);
   static ReplayInfo* ReadInfo(string replayPath);
   static string ToFilePath(Replay* replay);
};
