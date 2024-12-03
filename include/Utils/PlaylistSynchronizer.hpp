#include <map>
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <dirent.h>

using namespace std;
using namespace rapidjson;

class PlaylistSynchronizer
{
public:
   static string INSTALL_PATH;
   static void DownloadBeatmap(string path, string hash, int index, function<void(bool)> const &completion = nullptr);
   static void SyncPlaylist();
   static void GetBeatmap(int index);
   static void InstallPlaylist(string url, string filename, function<void(bool)> const &completion = nullptr);
};
