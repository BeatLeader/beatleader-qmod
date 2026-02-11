#pragma once

#include "shared/Models/Replay.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"

#include "UnityEngine/MonoBehaviour.hpp"

#include "custom-types/shared/macros.hpp"

#include "Zenject/ILateTickable.hpp"

#include <string>
#include <sstream>
#include <map>
#include <utility>

namespace GlobalNamespace {
class PlayerTransforms;
class SaberManager;
class AudioTimeSyncController;
}

using LevelEndStateType = GlobalNamespace::LevelCompletionResults::LevelEndStateType;
using LevelEndAction = GlobalNamespace::LevelCompletionResults::LevelEndAction;

enum class LevelEndType {
    Unknown = 0,
    Clear = 1,
    Fail = 2,
    Restart = 3,
    Quit = 4,
    Practice = 5
};

class PlayEndData {
public:
    PlayEndData(const GlobalNamespace::LevelCompletionResults* results, float speed) 
        : _endType(ToLevelEndAction(results, speed)),
          _time(results->endSongTime)
    {}

    PlayEndData(LevelEndType endType, float time)
        : _endType(endType), 
          _time(time)
    {}

    LevelEndType GetEndType() const { return _endType; }
    float GetTime() const { return _time; }

    std::string ToQueryString() const {
        if (_endType == LevelEndType::Clear) {
            return "";
        }

        std::ostringstream oss;
        oss << "?type=" << static_cast<int>(_endType) << "&time=" << _time;
        return oss.str();
    }

    PlayEndData& operator=(const PlayEndData& other) {
        if (this != &other) {
            _endType = other._endType;
            _time = other._time;
        }
        return *this;
    }

private:
    const std::map<std::pair<LevelEndStateType, LevelEndAction>, LevelEndType> _types = {
        { { LevelEndStateType::Failed    , LevelEndAction::Restart }, LevelEndType::Fail    },
        { { LevelEndStateType::Failed    , LevelEndAction::None    }, LevelEndType::Fail    },
        { { LevelEndStateType::Incomplete, LevelEndAction::Restart }, LevelEndType::Restart },
        { { LevelEndStateType::Incomplete, LevelEndAction::Quit    }, LevelEndType::Quit    },
        { { LevelEndStateType::Incomplete, LevelEndAction::None    }, LevelEndType::Quit    },
        { { LevelEndStateType::Cleared   , LevelEndAction::None    }, LevelEndType::Clear   }
    };

    LevelEndType ToLevelEndAction(const GlobalNamespace::LevelCompletionResults* results, float speed) {
        if (speed != 0) return LevelEndType::Practice;

        LevelEndStateType levelEndStateType = results->levelEndStateType;
        LevelEndAction levelEndAction = results->levelEndAction;

        auto it = _types.find({levelEndStateType, levelEndAction});
        return it != _types.end() ? it->second : LevelEndType::Unknown;
    }

    LevelEndType _endType;
    float _time;
};

namespace ReplayRecorder {
    void StartRecording(
        function<void(void)> const &startedCallback, 
        function<void(Replay const &, PlayEndData, bool)> const &callback);
    static bool recording;
}

DECLARE_CLASS_CODEGEN_INTERFACES(BeatLeader, ReplayRecorderTicker, UnityEngine::MonoBehaviour, Zenject::ILateTickable*) {
    private:
      GlobalNamespace::PlayerTransforms *playerTransforms;
      GlobalNamespace::SaberManager *saberManager;
      GlobalNamespace::AudioTimeSyncController* audioTimeSyncController;

      DECLARE_OVERRIDE_METHOD(void, LateTick, il2cpp_utils::il2cpp_type_check::MetadataGetter<&Zenject::ILateTickable::LateTick>::methodInfo());

      DECLARE_DEFAULT_CTOR();
      DECLARE_SIMPLE_DTOR();
  
public:
  void Init(GlobalNamespace::PlayerTransforms* playerTransforms, GlobalNamespace::SaberManager* saberManager, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController);
};