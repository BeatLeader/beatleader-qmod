#include "include/Managers/PrestigeLevelIconsManager.hpp"

#include "include/Core/Events.hpp"

#include "include/Utils/WebUtils.hpp"

#include "include/main.hpp"

#include "UnityEngine/Sprite.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/BSML-Lite/Creation/Image.hpp"

namespace BeatLeader {
    namespace PrestigeLevelIconsManagerNS {
        PrestigeLevelIconsManager Instance {};
    }

    UnityEngine::Sprite* PrestigeLevelIconsManager::getSprite(int level) {
        auto it = std::find_if(this->loadedSprites.begin(), this->loadedSprites.end(), [level](const auto& t) {
            return std::get<0>(t) == level;
        });
        if (this->loadedSprites.contains(level)) {
            return BSML::Lite::ArrayToSprite(ArrayW<uint8_t>(this->loadedSprites[level]));
        }
        else {
            // This may happen, if the leaderboard or scoredetails is opened before loading all icons. So we just use info
            BeatLeaderLogger.info("Could not find prestige icon {}", level);
            return NULL;
        }
    };

    void PrestigeLevelIconsManager::Init() {
        WebUtils::GetAsync(WebUtils::API_URL + "experience/levels", [this](long status, string response) {
            if (status != 200) {
                BeatLeaderLogger.warn("Failed prestige iconlist request");
                return;
            }

            rapidjson::Document levelsUrlsResult;
            levelsUrlsResult.Parse(response.c_str());
            if (levelsUrlsResult.HasParseError())
              return;
            auto levelsUrls = levelsUrlsResult.GetArray();
            const auto levelsUrlsLength = levelsUrls.Size();
            for(auto& element : levelsUrls) {
                const auto elementLevel = element["level"].GetInt();
                WebUtils::GetAsync(element["bigIcon"].GetString(), [elementLevel, levelsUrlsLength, this](long iconStatus, string iconResponse) {
                    if (iconStatus != 200) {
                        BeatLeaderLogger.warn("Failed prestige icon request {}", elementLevel);
                        return;
                    }
                    std::vector<uint8_t> iconBytes(iconResponse.begin(), iconResponse.end());


                    BSML::MainThreadScheduler::Schedule([elementLevel, levelsUrlsLength, iconBytes, this]() {
                        this->loadedSprites.insert_or_assign(elementLevel, iconBytes);

                        // If we have all levels, we can broadcast the loaded event
                        if (levelsUrlsLength == this->loadedSprites.size()) {
                            BeatLeader::EventManagement::Broadcast(BeatLeader::EventManagement::Events::PrestigeIconsLoaded);
                        }
                    });
                });
            }
        });
    };
}