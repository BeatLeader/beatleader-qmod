#include "include/Utils/ModifiersManager.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/constants.hpp"

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

map<string, float> ModifiersManager::modifiers;

void ModifiersManager::Sync() {
    string url = API_URL + "modifiers";

    WebUtils::GetJSONAsync(url, [] (long status, bool error, rapidjson::Document& result){
        for (auto i = result.MemberBegin(); i != result.MemberEnd(); ++i) {
            modifiers[i->name.GetString()] = i->value.GetFloat();
        }
    });
}