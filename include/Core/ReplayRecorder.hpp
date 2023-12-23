#pragma once

#include "include/Models/Replay.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"

#include <string>
#include <sstream>
#include <map>
#include <utility>

using LevelEndStateType = GlobalNamespace::LevelCompletionResults::LevelEndStateType;
using LevelEndAction = GlobalNamespace::LevelCompletionResults::LevelEndAction;

enum class LevelEndType {
    Unknown = 0,
    Clear = 1,
    Fail = 2,
    Restart = 3,
    Quit = 4
};

class PlayEndData {
public:
    PlayEndData(const GlobalNamespace::LevelCompletionResults* results) 
        : _endType(ToLevelEndAction(results)),
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

private:
    const std::map<std::pair<LevelEndStateType, LevelEndAction>, LevelEndType> _types = {
        { { LevelEndStateType::Failed    , LevelEndAction::Restart }, LevelEndType::Fail    },
        { { LevelEndStateType::Failed    , LevelEndAction::None    }, LevelEndType::Fail    },
        { { LevelEndStateType::Incomplete, LevelEndAction::Restart }, LevelEndType::Restart },
        { { LevelEndStateType::Incomplete, LevelEndAction::Quit    }, LevelEndType::Quit    },
        { { LevelEndStateType::Incomplete, LevelEndAction::None    }, LevelEndType::Quit    },
        { { LevelEndStateType::Cleared   , LevelEndAction::None    }, LevelEndType::Clear   }
    };

    LevelEndType ToLevelEndAction(const GlobalNamespace::LevelCompletionResults* results) {
        LevelEndStateType levelEndStateType = results->levelEndStateType;
        LevelEndAction levelEndAction = results->levelEndAction;

        auto it = _types.find({levelEndStateType, levelEndAction});
        return it != _types.end() ? it->second : LevelEndType::Unknown;
    }

    const LevelEndType _endType;
    const float _time;
};

namespace ReplayRecorder {
    void StartRecording(
        function<void(void)> const &startedCallback, 
        function<void(Replay const &, PlayEndData, bool)> const &callback);
    static bool recording;
}