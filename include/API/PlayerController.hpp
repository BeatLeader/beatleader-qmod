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

    static string LogIn(string login, string password);
    static string SignUp(string login, string password);
    static bool LogOut();

    static Player* currentPlayer;
    static Player* platformPlayer;
    static string lastErrorDescription;
};
