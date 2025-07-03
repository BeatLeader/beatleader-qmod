#include "Models/ScoreUpload.hpp"

ScoreUpload::ScoreUpload() {}

ScoreUpload::ScoreUpload(rapidjson::Value const& document) 
{
  Score = document["score"];
  Status = static_cast<ScoreUploadStatus>(document["status"].GetInt());
  Description = document["description"].GetString();
}