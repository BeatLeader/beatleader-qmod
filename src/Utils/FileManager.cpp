#include "include/main.hpp"
#include <cctype>
#include <filesystem>
#include <locale>
#include <fstream>
#include <codecvt>

#include "include/Utils/FileManager.hpp"
#include "include/Utils/ModConfig.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"

namespace {
    string GetReplaysDirectory() {
        return getDataDir(modInfo) + "replays/";
    }

    string CollapseWhitespace(string const& value) {
        string result;
        result.reserve(value.size());

        bool pendingSpace = false;
        for (unsigned char current : value) {
            if (std::isspace(current) != 0) {
                if (!result.empty()) {
                    pendingSpace = true;
                }
                continue;
            }

            if (pendingSpace) {
                result.push_back(' ');
                pendingSpace = false;
            }

            result.push_back(static_cast<char>(current));
        }

        return result;
    }

    string SanitizeFilenameComponent(string const& value, bool removeDashes, string const& fallback) {
        static constexpr string_view illegalChars = "\\/:?*\"<>()|\r\n";

        string filtered;
        filtered.reserve(value.size());

        for (unsigned char current : value) {
            if (current < 32 || current >= 127) {
                continue;
            }

            char character = static_cast<char>(current);
            if (removeDashes && character == '-') {
                continue;
            }

            filtered.push_back(illegalChars.find(character) != string_view::npos ? '_' : character);
        }

        auto sanitized = CollapseWhitespace(filtered);
        return sanitized.empty() ? fallback : sanitized;
    }
}

void FileManager::EnsureReplaysFolderExists() {
    string directory = GetReplaysDirectory();
    filesystem::create_directories(directory);
}

void FileManager::WriteReplay(string const& replayPath, Replay const &replay) {
    ofstream stream = ofstream(replayPath, ios::binary);

    std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
    stream.imbue(utf8_locale);

    replay.Encode(stream);

    stream.flush();
}

std::optional<ReplayInfo> FileManager::ReadInfo(string replayPath) {
    ifstream stream(replayPath, ios::binary);

    std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
    stream.imbue(utf8_locale);

    return Replay::DecodeInfo(stream);
}

string FileManager::ToFilePath(Replay const &replay, PlayEndData const& playEndData) {
    string practice = replay.info.speed > 0.0001 ? "-practice" : "";
    string fail = replay.info.failTime > 0.0001 ? "-fail" : "";
    string exit = playEndData.GetEndType() == LevelEndType::Quit || playEndData.GetEndType() == LevelEndType::Restart ? "-exit" : "";

    string filename = SanitizeFilenameComponent(replay.info.playerID, false, "UnknownPlayer")
        + practice + fail + exit
        + "-" + SanitizeFilenameComponent(replay.info.songName, true, "UnknownSong")
        + "-" + SanitizeFilenameComponent(replay.info.difficulty, false, "UnknownDifficulty")
        + "-" + SanitizeFilenameComponent(replay.info.mode, false, "UnknownMode")
        + "-" + SanitizeFilenameComponent(replay.info.hash, false, "UnknownHash");

    if (getModConfig().SaveEveryReplayAttempt.GetValue()) {
        auto timestamp = SanitizeFilenameComponent(replay.info.timestamp, false, "");
        if (!timestamp.empty()) {
            filename += "-" + timestamp;
        }
    }

    return GetReplaysDirectory() + filename + ".bsor";
}