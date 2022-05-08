#pragma once
#include <string>

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

namespace WebUtils {
    extern std::string GameVersion;
    extern std::string API_URL;
    extern std::string WEB_URL;

    void refresh_urls();

    std::optional<rapidjson::Document> GetJSON(std::string_view url);

    long Get(std::string_view url, std::string& val);

    long Get(std::string_view url, long timeout, std::string& val);

    void GetAsync(std::string_view url, std::function<void(long, std::string_view)> const &finished, std::function<void(
            float)> const &progressUpdate = nullptr);

    void GetAsync(std::string_view url, long timeout, std::function<void(long, std::string_view)> const& finished, std::function<void(float)> const& progressUpdate = nullptr);

    void GetJSONAsync(std::string_view url, std::function<void(long, bool, rapidjson::Document const&)> const& finished);

    void PostJSONAsync(std::string_view url, std::string_view data, std::function<void(long, std::string_view)> const& finished);

    void PostJSONAsync(const std::string& url, std::string data, long timeout, std::function<void(long, std::string_view)> const& finished);

    void PostFormAsync(const std::string& url, const std::string& password, const std::string& login, const std::string& action,
                       std::function<void(long, std::string_view)> const &finished);

    void PostFileAsync(std::string_view url, FILE* data, long length, long timeout, std::function<void(long, std::string_view)> const& finished, std::function<void(float)> const& progressUpdate);
}