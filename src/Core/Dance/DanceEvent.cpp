#include "include/Core/Dance/DanceEvent.hpp"
#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Utils/StringUtils.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

#include "shared/Models/Paged.hpp"

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include <string_view>
#include <unordered_set>

namespace DanceEvent {
    static constexpr size_t HashLength = 40;
    static constexpr std::string_view CustomLevelPrefix = "custom_level_";
    static constexpr std::string_view Bom = "\xEF\xBB\xBF";

    static std::optional<PlatformEvent> current;
    static std::vector<std::function<void()>> listeners;
    static bool loading = false;
    static bool loaded = false;

    static std::unordered_set<std::string> maps;
    static bool mapsLoaded = false;
    static bool mapsLoading = false;

    static std::string MapKey(std::string_view hash, std::string_view characteristic, std::string_view difficulty) {
        return toLower(std::string(hash) + "|" + std::string(characteristic) + "|" + std::string(difficulty));
    }

    std::optional<PlatformEvent> const& Current() {
        return current;
    }

    bool IsEventLevel(GlobalNamespace::BeatmapKey const& key) {
        if (!current || !current->IsHappening() || maps.empty()) return false;
        if (!key.levelId) return false;
        std::string levelId = key.levelId;
        if (!levelId.starts_with(CustomLevelPrefix) || levelId.size() < CustomLevelPrefix.size() + HashLength) return false;
        auto hash = levelId.substr(CustomLevelPrefix.size(), HashLength);
        if (maps.contains(toLower(hash))) return true;

        auto characteristicSO = key.beatmapCharacteristic;
        std::string characteristic = characteristicSO ? (std::string)characteristicSO->serializedName : "";
        return maps.contains(MapKey(hash, characteristic, MapEnhancer::DiffName(key.difficulty.value__)));
    }

    static std::optional<std::unordered_set<std::string>> ParseMaps(std::string body) {
        if (body.starts_with(Bom)) body.erase(0, Bom.size());
        rapidjson::Document playlist;
        playlist.Parse(body.data());
        if (playlist.HasParseError() || !playlist.IsObject()) return std::nullopt;

        std::unordered_set<std::string> result;
        auto songs = playlist.FindMember("songs");
        if (songs == playlist.MemberEnd() || !songs->value.IsArray()) return result;
        for (auto const& song : songs->value.GetArray()) {
            if (!song.IsObject()) continue;
            auto hashMember = song.FindMember("hash");
            if (hashMember == song.MemberEnd() || !hashMember->value.IsString()) continue;
            std::string hash = hashMember->value.GetString();
            if (hash.size() < HashLength) continue;
            hash = hash.substr(0, HashLength);

            auto difficulties = song.FindMember("difficulties");
            if (difficulties == song.MemberEnd() || !difficulties->value.IsArray() || difficulties->value.Empty()) {
                result.insert(toLower(hash));
                continue;
            }
            for (auto const& difficulty : difficulties->value.GetArray()) {
                if (!difficulty.IsObject()) continue;
                auto name = difficulty.FindMember("name");
                if (name == difficulty.MemberEnd() || !name->value.IsString()) continue;
                auto characteristic = difficulty.FindMember("characteristic");
                std::string characteristicName = characteristic != difficulty.MemberEnd() && characteristic->value.IsString()
                    ? characteristic->value.GetString()
                    : "Standard";
                result.insert(MapKey(hash, characteristicName, name->value.GetString()));
            }
        }
        return result;
    }

    static void LoadMaps(PlatformEvent const& evt) {
        mapsLoading = true;
        int eventId = evt.id;
        std::string eventName = evt.name;
        WebUtils::GetAsync(WebUtils::API_URL + "playlist/" + std::to_string(evt.playlistId), [eventId, eventName](long status, std::string body) {
            auto parsed = status == 200 ? ParseMaps(std::move(body)) : std::nullopt;
            BSML::MainThreadScheduler::Schedule([eventId, eventName, status, parsed = std::move(parsed)] {
                if (!current || current->id != eventId) return;
                mapsLoading = false;
                if (!parsed) {
                    BeatLeaderLogger.warn("Couldn't load the maps of {}: {}", eventName, status);
                    return;
                }
                maps = std::move(*parsed);
                mapsLoaded = true;
                BeatLeaderLogger.info("{}: {} event difficulties", eventName, maps.size());
            });
        });
    }

    void Load() {
        if (current) {
            if (!mapsLoaded && !mapsLoading) LoadMaps(*current);
            return;
        }
        if (loading || loaded) return;
        loading = true;
        WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/events", [](long status, bool error, rapidjson::Document const& response) {
            std::optional<std::vector<PlatformEvent>> events;
            if (status == 200 && !error && response.IsObject() && response.HasMember("data") && response["data"].IsArray()) {
                try {
                    events = Paged<PlatformEvent>(response.GetObject()).data;
                } catch (std::exception const& e) {
                    BeatLeaderLogger.error("Couldn't read the events: {}", e.what());
                }
            }
            BSML::MainThreadScheduler::Schedule([events = std::move(events)] {
                loading = false;
                if (events) SetEvents(*events);
            });
        });
    }

    void SetEvents(std::vector<PlatformEvent> const& events) {
        loaded = true;
        std::optional<PlatformEvent> found;
        for (auto const& evt : events) {
            if (evt.eventType == EventType && evt.IsHappening()) {
                found = evt;
                break;
            }
        }

        bool changed = (found ? found->id : -1) != (current ? current->id : -1);
        current = found;
        if (!changed) return;

        maps.clear();
        mapsLoaded = false;
        mapsLoading = false;
        if (current) LoadMaps(*current);

        auto copy = listeners;
        for (auto const& listener : copy) {
            listener();
        }
    }

    void AddCurrentChangedListener(std::function<void()> listener) {
        listeners.push_back(std::move(listener));
    }
}
