#include <map>
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <dirent.h>

using namespace std;
using namespace rapidjson;

enum ReplayStatus {
    unknown = 0,
    uptodate = 1,
    shouldnotpost = 2,
    topost = 3
};

class ReplaySynchronizer
{
public:
    ReplaySynchronizer() noexcept;
    void updateStatus(string path, ReplayStatus status);

    void Save();
    void Process(DIR *dir, string dirName);
    Document statuses;
};
