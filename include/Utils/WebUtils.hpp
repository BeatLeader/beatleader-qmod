#pragma once
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

using namespace std;

namespace WebUtils {
    extern string GameVersion;
    extern string API_URL;
    extern string WEB_URL;

    void refresh_urls();

    string getCookieFile();

    std::optional<rapidjson::Document> GetJSON(string url);

    long Get(string url, string& val);

    long Get(string url, long timeout, string& val);

    std::thread RequestAsync(string url, string method, long timeout, function<void(long, string)> const &finished, function<void(
            float)> const &progressUpdate = nullptr);
    std::thread GetAsync(string url, function<void(long, string)> const &finished, function<void(
            float)> const &progressUpdate = nullptr);

    std::thread GetAsync(string url, long timeout, function<void(long, string)> const& finished, function<void(float)> const& progressUpdate = nullptr);
    std::thread GetAsyncFile(string url, string path, long timeout, const function<void(long)>& finished, const function<void(float)>& progressUpdate = nullptr);
    std::thread GetJSONAsync(string url, function<void(long, bool, rapidjson::Document const&)> const& finished);

    std::thread PostJSONAsync(string url, string data, function<void(long, string)> const& finished);

    std::thread PostJSONAsync(const string& url, string data, long timeout, function<void(long, string)> const& finished);

    std::thread PostFormAsync(const string& url, const string& password, const string& login, const string& action,
                       function<void(long, string)> const &finished);

    std::thread PostFileAsync(string url, FILE* data, long length, function<void(long, string, string)> const& finished, function<void(float)> const& progressUpdate);
}