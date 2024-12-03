#include "include/Managers/LeaderboardContextsManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Assets/Sprites.hpp"
#include "main.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

namespace BeatLeader {
    void LeaderboardContextsManager::UpdateContexts(function<void()> const &completion) {
        WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/leaderboardContexts", [completion](long status, bool error, rapidjson::Document const& doc) {
            if (status != 200 || error) {
                BeatLeaderLogger.error("Failed to get contexts: %ld", status);
                return;
            }

            std::vector<ScoresContext*> newContexts = {};
            
            for (const auto& contextJson : doc.GetArray()) {
                auto context = new ScoresContext(contextJson);

                if (contextJson.HasMember("icon") && !contextJson["icon"].IsNull()) {
                    Sprites::get_Icon(contextJson["icon"].GetString(), [context](UnityEngine::Sprite* sprite) {
                        if (sprite) {
                            context->icon = sprite;
                            if (context->iconCallback) context->iconCallback();
                        }
                    });
                }

                newContexts.push_back(context);
            }

            BSML::MainThreadScheduler::Schedule([newContexts, completion]() {
                ScoresContexts::allContexts = newContexts;
                if (completion) completion();
            });
        });
    }
} 