#pragma onece
#include "Score.hpp"
#include <string>

enum class ScoreUploadStatus {
    Uploaded = 1,
    NonPB = 2,
    Attempt = 3,
    Error = 4,
};

struct ScoreUpload 
{
    Score Score;
    ScoreUploadStatus Status;
    std::string Description;

    ScoreUpload();
    ScoreUpload(rapidjson::Value const& document);
};