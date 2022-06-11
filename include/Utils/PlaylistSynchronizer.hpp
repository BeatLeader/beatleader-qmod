#include <map>
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <dirent.h>

using namespace std;
using namespace rapidjson;

class PlaylistSynchronizer
{
public:
   static void SyncPlaylist();
};
