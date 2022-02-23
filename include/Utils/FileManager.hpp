#pragma once

#include "include/Models/Replay.hpp"

class FileManager
{
private:
    /* data */
public:
   static void WriteReplay(Replay* replay);
private:
   static string ToFileName(Replay* replay);
};
