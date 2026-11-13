#include "shared/Models/PlatformEvent.hpp"

#include <chrono>

PlatformEvent::PlatformEvent(rapidjson::Value const& document) {
    if (document.HasMember("id") && document["id"].IsInt()) {
        id = document["id"].GetInt();
    }
    image = document["image"].GetString();
    name = document["name"].GetString();
    endDate = document["endDate"].GetInt64();
    downloadable = document["downloadable"].GetBool();
    playerCount = document["playerCount"].GetInt();
    playlistId = document["playlistId"].GetInt();
    if (document.HasMember("eventType") && document["eventType"].IsInt()) {
        eventType = document["eventType"].GetInt();
    }

    if (document.HasMember("description") && !document["description"].IsNull()) {
        description = document["description"].GetString();
    } else {
        description = "";
    }
}

bool PlatformEvent::IsHappening() const {
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return now < endDate;
}
