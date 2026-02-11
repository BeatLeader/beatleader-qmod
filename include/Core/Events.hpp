#include <functional>
#include <string>
#include <string_view>
#include "metacore/shared/events.hpp"
#include "main.hpp"

namespace BeatLeader::EventManagement {
    static constexpr std::string_view EVENT_IDENTIFIER = "BeatLeader";
    enum class Events {
        PrestigeIconsLoaded,
    };

    static void RegisterEvent(BeatLeader::EventManagement::Events event) {
        BeatLeaderLogger.warn("EVENT REGISTER {}", static_cast<int>(event));
        MetaCore::Events::RegisterEvent(std::string(BeatLeader::EventManagement::EVENT_IDENTIFIER), static_cast<int>(event));
    };

    static int AddCallback(BeatLeader::EventManagement::Events event, std::function<void()> function, bool once = false) {
        BeatLeaderLogger.warn("EVENT ADDCB {}", static_cast<int>(event));
        auto ret = MetaCore::Events::AddCallback(std::string(BeatLeader::EventManagement::EVENT_IDENTIFIER), static_cast<int>(event), function, once);
        BeatLeaderLogger.warn("EVENT ADDCB {}", static_cast<int>(ret));
        return ret;
    };

    static bool Broadcast(BeatLeader::EventManagement::Events event) {
        BeatLeaderLogger.warn("EVENT BROADCAST {}", static_cast<int>(event));
        return MetaCore::Events::Broadcast(std::string(BeatLeader::EventManagement::EVENT_IDENTIFIER), static_cast<int>(event));
    };

    static void RegisterAllEvents() {
        BeatLeader::EventManagement::RegisterEvent(BeatLeader::EventManagement::Events::PrestigeIconsLoaded);
    };
}