#include "Models/ScoreUpload.hpp"

ScoreUpload::ScoreUpload() {}

ScoreUpload::ScoreUpload(rapidjson::Value const& document) 
{
  if (document.HasMember("score") && !document["score"].IsNull()) {
    score = Score(document["score"]);
  } else {
    score = nullopt;
  }
  status = static_cast<ScoreUploadStatus>(document["status"].GetInt());
  description = document["description"].GetString();
}