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

Difficulty::Difficulty(float starsGiven, int statusGiven, int typeGiven, vector<float> votesGiven) {
    stars = starsGiven;
    status = statusGiven;
    type = typeGiven;
    votes = votesGiven;
}