#pragma once

#include <string>
#include "UnityEngine/Sprite.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

namespace BeatLeader {
    class ScoresContext {
        public:
            int id;
            std::string name;
            std::string description; 
            std::string key;
            std::string icon;

            ScoresContext() : id(0) {}
            ScoresContext(int id, std::string name, std::string icon, std::string description, std::string key) 
                : id(id), name(name), icon(icon), description(description), key(key) {}
            ScoresContext(rapidjson::Value const& document);
    };

    class ScoresContexts {
        public:
            static std::vector<ScoresContext*> allContexts;
            static ScoresContext* general;
            static ScoresContext* getContextForId(int id);
            static void initializeGeneral();
    };
} 