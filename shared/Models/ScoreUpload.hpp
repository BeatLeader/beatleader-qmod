#pragma once

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
    std::optional<Score> score;
    ScoreUploadStatus status;
    std::string description;

    ScoreUpload();
    ScoreUpload(rapidjson::Value const& document);
};