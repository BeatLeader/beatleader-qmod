#include <map>
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include <dirent.h>

#include "Utils/ThreadKeeper.hpp"

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

    static void updateStatus(string path, ReplayStatus status, rapidjson::Document &doc);

    static void Save(rapidjson::Document const &doc) ;
    static void Process(DIR *dir, string dirName, shared_ptr<Document> docPtr);

    std::shared_ptr<Document> statuses;
};
