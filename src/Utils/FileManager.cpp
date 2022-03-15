#include "include/main.hpp"

#include <filesystem>
#include <locale>
#include <fstream>
#include <regex>

#include "include/Utils/FileManager.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"

void FileManager::WriteReplay(Replay* replay) {
    ofstream stream = ofstream(ToFilePath(replay), ios::binary);

    std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
    stream.imbue(utf8_locale);

    replay->Encode(stream);

    stream.flush();
}

ReplayInfo* FileManager::ReadInfo(string replayPath) {
    ifstream stream = ifstream(replayPath, ios::binary);

    std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
    stream.imbue(utf8_locale);

    return Replay::DecodeInfo(stream);
}

string FileManager::ToFilePath(Replay* replay) {
    string practice = replay->info->speed > 0.0001 ? "-practice" : "";
    string fail = replay->info->failTime > 0.0001 ? "-fail" : "";
    string filename = replay->info->playerID + practice + fail + "-" + replay->info->songName + "-" + replay->info->difficulty + "-" + replay->info->mode + "-" + replay->info->hash + ".bsor";
    string directory = getDataDir(modInfo) + "replays/";
    filesystem::create_directories(directory);
    string file = directory + regex_replace(filename, basic_regex("/"), "");;
    // string regexSearch = new string(System::IO::Path::GetInvalidFileNameChars()) + new string(System::IO::Path::GetInvalidPathChars());
    // string reg = "[{" + regexSearch + "}]";
    return file;// regex_replace(filename, basic_regex(reg), "_");
}