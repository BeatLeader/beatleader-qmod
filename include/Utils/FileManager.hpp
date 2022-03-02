#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

class FileManager
{
private:
    /* data */
public:
   static string WriteReplay(Replay* replay);
private:
   static string ToFileName(Replay* replay);
};
