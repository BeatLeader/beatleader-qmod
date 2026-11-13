#include "Models/ScoreUpload.hpp"

ScoreUpload::ScoreUpload() {}

ScoreUpload::ScoreUpload(rapidjson::Value const& document) 
{
  status = ScoreUploadStatus::Error;
  if (!document.IsObject()) {
    description = "Unexpected server response";
    return;
  }
  if (document.HasMember("score") && document["score"].IsObject()) {
    score = Score(document["score"]);
  } else {
    score = nullopt;
  }
  if (document.HasMember("status") && document["status"].IsInt()) {
    status = static_cast<ScoreUploadStatus>(document["status"].GetInt());
  } else if (document.HasMember("status") && document["status"].IsString()) {
    std::string name = document["status"].GetString();
    if (name == "Uploaded") status = ScoreUploadStatus::Uploaded;
    else if (name == "NonPB") status = ScoreUploadStatus::NonPB;
    else if (name == "Attempt") status = ScoreUploadStatus::Attempt;
  }
  description = document.HasMember("description") && document["description"].IsString()
    ? document["description"].GetString()
    : "";
}