#include "include/main.hpp"

#include <filesystem>
#include <locale>
#include <fstream>

#include "include/Utils/FileManager.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"

string FileManager::WriteReplay(Replay* replay) {
    string directory = getDataDir(modInfo) + "replays/";
    filesystem::create_directories(directory);
    string file = directory + ToFileName(replay);

    ofstream stream = ofstream(file, ios::binary);

    std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
    stream.imbue(utf8_locale);

    replay->Encode(stream);

    stream.flush();

    return file;
}

string FileManager::ToFileName(Replay* replay) {
    string practice = replay->info->speed > 0.0001 ? "-practice" : "";
    string fail = replay->info->failTime > 0.0001 ? "-fail" : "";
    string filename = replay->info->playerID + practice + fail + "-" + replay->info->songName + "-" + replay->info->difficulty + "-" + replay->info->mode + "-" + replay->info->hash + ".bsor";
    // string regexSearch = new string(System::IO::Path::GetInvalidFileNameChars()) + new string(System::IO::Path::GetInvalidPathChars());
    // string reg = "[{" + regexSearch + "}]";
    return filename;// regex_replace(filename, basic_regex(reg), "_");
}