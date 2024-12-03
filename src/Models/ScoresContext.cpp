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
        icon = nullptr;
        iconCallback = nullptr;
    }

    void ScoresContexts::initializeGeneral() {
        general = new ScoresContext(0, "General", "General", "modifiers");
        allContexts = {general};
    }

    ScoresContext* ScoresContexts::getContextForId(int id) {
        auto it = std::find_if(allContexts.begin(), allContexts.end(), 
            [id](const ScoresContext* context) { return context->id == id; });
        return it != allContexts.end() ? *it : general;
    }
}  