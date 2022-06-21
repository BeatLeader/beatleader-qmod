#include "Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
    static string RefreshOnline();
public:
    static string Refresh();
    static vector<function<void(optional<Player> const&)>> playerChanged;

    static void LogIn(string login, string password, const function<void(string)>& finished);
    static void SignUp(string login, string password, const function<void(string)>& finished);
    static bool LogOut();

    static optional<Player> currentPlayer;
    static string lastErrorDescription;
};
