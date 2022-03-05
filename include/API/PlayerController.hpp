#include "Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
public:
    static string Refresh();
    static function<void(Player*)> playerChanged;

    static string LogIn(string token);
    static bool LogOut();

    static Player* currentPlayer;
    static string lastErrorDescription;
};
