#include "include/Managers/MapTypesManager.hpp"

#include "include/Utils/WebUtils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "main.hpp"

namespace BeatLeader {
    namespace MapTypesManagerNS {
        MapTypesManager Instance {};
    }

    void MapTypesManager::Init() {
        if (_initialized) {
            return;
        }

        _initialized = true;
        WebUtils::GetAsync(WebUtils::API_URL + "mod/mapTypes", [this](long status, std::string response) {
            if (status != 200) {
                BeatLeaderLogger.warn("Failed map types request");
                return;
            }

            rapidjson::Document result;
            result.Parse(response.c_str());
            if (result.HasParseError() || !result.IsArray()) {
                BeatLeaderLogger.warn("Failed to parse map types response");
                return;
            }

            std::vector<MapsTypeDescription> mapTypes;
            mapTypes.reserve(result.Size());
            for (auto const& element : result.GetArray()) {
                mapTypes.emplace_back(element);
            }

            BSML::MainThreadScheduler::Schedule([this, mapTypes = std::move(mapTypes)]() mutable {
                _mapTypes = std::move(mapTypes);
                _loaded = true;
                NotifyCallbacks();
            });
        });
    }

    bool MapTypesManager::IsLoaded() const {
        return _loaded;
    }

    std::vector<MapsTypeDescription> const& MapTypesManager::GetMapTypes() const {
        return _mapTypes;
    }

    int MapTypesManager::AddCallback(std::function<void()> callback, bool invokeImmediately) {
        auto callbackId = _nextCallbackId++;
        _callbacks.insert_or_assign(callbackId, std::move(callback));

        if (invokeImmediately && _loaded) {
            auto it = _callbacks.find(callbackId);
            if (it != _callbacks.end() && it->second) {
                it->second();
            }
        }

        return callbackId;
    }

    void MapTypesManager::RemoveCallback(int callbackId) {
        _callbacks.erase(callbackId);
    }

    void MapTypesManager::NotifyCallbacks() {
        auto callbacks = _callbacks;
        for (auto const& [_, callback] : callbacks) {
            if (callback) {
                callback();
            }
        }
    }
}
