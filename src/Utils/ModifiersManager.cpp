#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/WebUtils.hpp"

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

unordered_map<string, float> ModifiersManager::modifiers;

void ModifiersManager::Sync() {
    string url = WebUtils::API_URL + "modifiers";

    WebUtils::GetJSONAsync(url, [] (long status, bool error, rapidjson::Document const& result){
        if (status == 200 && !error) {
            for (auto i = result.MemberBegin(); i != result.MemberEnd(); ++i) {
                modifiers[i->name.GetString()] = i->value.GetFloat();
            }
        }
    });
}