#include "shared/Models/PlatformEvent.hpp"

PlatformEvent::PlatformEvent(rapidjson::Value const& document) {
    image = document["image"].GetString();
    name = document["name"].GetString();
    endDate = document["endDate"].GetInt64();
    downloadable = document["downloadable"].GetBool();
    playerCount = document["playerCount"].GetInt();
    playlistId = document["playlistId"].GetInt();
    
    if (document.HasMember("description") && !document["description"].IsNull()) {
        description = document["description"].GetString();
    } else {
        description = "";
    }
} 