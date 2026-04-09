#pragma once

#include "shared/Models/MapsTypeDescription.hpp"

#include <functional>
#include <map>
#include <vector>

namespace BeatLeader {

class MapTypesManager {
public:
    void Init();

    [[nodiscard]] bool IsLoaded() const;
    [[nodiscard]] std::vector<MapsTypeDescription> const& GetMapTypes() const;

    int AddCallback(std::function<void()> callback, bool invokeImmediately = false);
    void RemoveCallback(int callbackId);

private:
    void NotifyCallbacks();

    bool _initialized = false;
    bool _loaded = false;
    int _nextCallbackId = 1;
    std::vector<MapsTypeDescription> _mapTypes;
    std::map<int, std::function<void()>> _callbacks;
};

namespace MapTypesManagerNS {
    extern MapTypesManager Instance;
}

}
