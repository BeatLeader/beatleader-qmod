#include "include/Models/ScoresContext.hpp"
#include "include/Assets/BundleLoader.hpp"
#include "include/Assets/Sprites.hpp"

namespace BeatLeader {
    std::vector<ScoresContext*> ScoresContexts::allContexts;
    ScoresContext* ScoresContexts::general;

    ScoresContext::ScoresContext(rapidjson::Value const& document) {
        id = document["id"].GetInt();
        name = document["name"].GetString();
        description = document["description"].GetString();
        key = document["key"].GetString();
        icon = document["icon"].GetString();
    }

    void ScoresContexts::initializeGeneral() {
        general = new ScoresContext(0, "General", "", "General", "modifiers");
        allContexts = {general};
    }

    ScoresContext* ScoresContexts::getContextForId(int id) {
        for (auto context : allContexts) {
            if (context->id == id) return context;
        }
        return general;
    }
}  