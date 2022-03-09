#include "Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
    static void RefreshPlatform();
    static string RefreshOnline();
public:
    static string Refresh();
    static function<void(Player*)> playerChanged;

    static void LogIn(string login, string password, std::function<void(std::string)> finished);
    static void SignUp(string login, string password, std::function<void(std::string)> finished);
    static bool LogOut();

    static Player* currentPlayer;
    static Player* platformPlayer;
    static string lastErrorDescription;
};
