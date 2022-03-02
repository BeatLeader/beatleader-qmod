#pragma once

#include "include/Models/Replay.hpp"

using namespace std;

#include <string>

class ReplayManager
{
private:
   static void TryPostReplay(string name, int tryIndex);
public:
   static void ProcessReplay(Replay* replay);
};
