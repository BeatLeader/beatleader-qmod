#include "include/Models/Difficulty.hpp"
#include "include/Models/TriangleRating.hpp"

Difficulty::Difficulty(rapidjson::Value const& document) {
    status = document["status"].GetInt();
    type = document["type"].GetInt();

    auto receivedVotes = document["votes"].GetArray();

    for(const auto& vote : receivedVotes) {
        votes.push_back(vote.GetFloat());
    }

    auto receivedModifierValues = document["modifierValues"].GetObject();

    for(auto& kv : receivedModifierValues){
        string key = kv.name.GetString();
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        modifierValues[key] = kv.value.GetFloat();
    }

    auto receivedModifiersRating = document["modifiersRating"].GetObject();

    for(auto& kv : receivedModifiersRating){
        string key = kv.name.GetString();
        if(key == "id"){
            continue;
        }

        // Extract aspect (tech, acc, pass, stars) and shorten key (to fs/sf/ss)
        string aspect = key.substr(2);
        key = key.substr(0, 2);

        std::transform(key.begin(), key.end(), key.begin(), ::toupper);

        float value = kv.value.GetFloat();
        if (aspect == "Stars")
            modifiersRating[key].stars = value;
        else if (aspect == "TechRating")
            modifiersRating[key].techRating = value;
        else if (aspect == "AccRating")
            modifiersRating[key].accRating = value;
        else if (aspect == "PassRating")
            modifiersRating[key].passRating = value;
        else if (aspect == "PredictedAcc"){}
        else
            modifiersRating[key].stars = -1;
    }

    rating.stars = document["stars"].GetFloat();
    rating.passRating = document["passRating"].GetFloat();
    rating.accRating = document["accRating"].GetFloat();
    rating.techRating = document["techRating"].GetFloat();
}

Difficulty::Difficulty(int statusGiven, int typeGiven, vector<float> votesGiven, unordered_map<string, float> modifierValuesGiven,unordered_map<string, TriangleRating> modifiersRatingGiven, TriangleRating ratingGiven) {
    status = statusGiven;
    type = typeGiven;
    votes = votesGiven;
    modifierValues = modifierValuesGiven;
    modifiersRating = modifiersRatingGiven;
    rating = ratingGiven;
}