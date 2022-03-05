#include "include/API/PlayerController.hpp"
#include "include/Utils/constants.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/main.hpp"

Player* PlayerController::currentPlayer = NULL;
string PlayerController::lastErrorDescription = "";
function<void(Player*)> PlayerController::playerChanged = [](Player* player) {

};

string PlayerController::Refresh() {
    string result = "";
    WebUtils::Get(API_URL + "user/id", result);
    if (result.length() > 0) {
        currentPlayer = new Player();
        currentPlayer->id = result;

        WebUtils::GetJSONAsync(API_URL + "player/" + result, [](long status, bool error, rapidjson::Document& result){
            if (status == 200) {
                currentPlayer = new Player(result);
                playerChanged(currentPlayer);
            }
        });
    }
    return result;
}

string PlayerController::LogIn(string token) {
    string result = "";
    lastErrorDescription = "";
    if (token.length() == 144) {
        string error = "";
        long statusCode = WebUtils::Get(API_URL + "signinoculus?token=" + token, error);
        if (statusCode == 200) {
            result = Refresh();
        } else {
            lastErrorDescription = error;
            getLogger().error("BeatLeader %s", ("signin error" + to_string(statusCode)).c_str());
        }
    } else {
        lastErrorDescription = "Code is not of valid length";
    }

    return result;
}

bool PlayerController::LogOut() {
    string result = "";
    WebUtils::Get(API_URL + "signout", result);
    WebUtils::Get(API_URL + "user/id", result);
    if (result.length() == 0) {
        currentPlayer = NULL;
        playerChanged(currentPlayer);
        return true;
    } else {
        return false;
    }
}