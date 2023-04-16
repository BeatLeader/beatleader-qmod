#include "include/main.hpp"

#include <filesystem>
#include <locale>
#include <fstream>
#include <regex>

#include "include/Utils/FileManager.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"

void FileManager::EnsureReplaysFolderExists() {
    string directory = getDataDir(modInfo) + "replays/";
    filesystem::create_directories(directory);
}

void FileManager::WriteReplay(Replay const &replay) {
    ofstream stream = ofstream(ToFilePath(replay), ios::binary);

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

string FileManager::ToFilePath(Replay const &replay) {
    string practice = replay.info.speed > 0.0001 ? "-practice" : "";
    string fail = replay.info.failTime > 0.0001 ? "-fail" : "";
    // TODO: Use fmt
    string filename = replay.info.playerID + std::string(practice + fail) + "-" + replay.info.songName + "-" + replay.info.difficulty + "-" + replay.info.mode + "-" + replay.info.hash + ".bsor";

    string illegalChars = "\\/:?*\"<>|";
    for (auto it = filename.begin() ; it < filename.end() ; ++it){
        bool found = illegalChars.find(*it) != string::npos;
        if(found){
            *it = '';
        }
    }
    string file = getDataDir(modInfo) + "replays/" + regex_replace(filename, basic_regex("/"), "");;
    // string regexSearch = new string(System::IO::Path::GetInvalidFileNameChars()) + new string(System::IO::Path::GetInvalidPathChars());
    // string reg = "[{" + regexSearch + "}]";
    return file;// regex_replace(filename, basic_regex(reg), "_");
}