#include "include/Models/Difficulty.hpp"
#include "include/Models/TriangleRating.hpp"

Difficulty::Difficulty(rapidjson::Value const& document) {
    status = document["status"].GetInt();
    type = document["type"].GetInt();

    // Load votes
    auto receivedVotes = document["votes"].GetArray();
    for(const auto& vote : receivedVotes) {
        votes.push_back(vote.GetFloat());
    }

    // Load ModifierValues (% increase or decrease for specific modifiers on this diff)
    auto receivedModifierValues = document["modifierValues"].GetObject();
    for(auto& kv : receivedModifierValues){
        string key = kv.name.GetString();
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        modifierValues[key] = kv.value.GetFloat();
    }

    // Load different ratings for modifiers
    auto receivedModifiersRating = document["modifiersRating"].GetObject();
    for(auto& kv : receivedModifiersRating){
        string key = kv.name.GetString();
        // We dont need id and our substring would crash with that
        if(key == "id" || key.length() < 3){
            continue;
        }

        // Extract aspect (tech, acc, pass, stars) and shorten key (to fs/sf/ss)
        string aspect = key.substr(2);
        key = key.substr(0, 2);

        // Normalize key
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);

        // Load our the aspect into our model
        float value = kv.value.GetFloat();
        if (aspect == "Stars")
            modifiersRating[key].stars = value;
        else if (aspect == "TechRating")
            modifiersRating[key].techRating = value;
        else if (aspect == "AccRating")
            modifiersRating[key].accRating = value;
        else if (aspect == "PassRating")
            modifiersRating[key].passRating = value;
        // Not used ingame for now (its "predicted average accuracy of top 10 players on this map")
        else if (aspect == "PredictedAcc"){}
        // Should never happen but so we notice when something goes wrong
        else
            modifiersRating[key].stars = -1;
    }

    // Load non modifier rating for this diff
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