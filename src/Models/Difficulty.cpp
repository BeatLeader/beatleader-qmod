#include "include/Models/Difficulty.hpp"

Difficulty::Difficulty(rapidjson::Value const& document) {
    stars = document["stars"].GetFloat();
    status = document["status"].GetInt();
    type = document["type"].GetInt();

    auto receivedVotes = document["votes"].GetArray();

    for(const auto& vote : receivedVotes) {
        votes.push_back(vote.GetFloat());
    }
}