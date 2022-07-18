#include "Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
public:
    static void Refresh(const function<void(optional<Player> const&, string)>& finished = nullptr);
    static vector<function<void(optional<Player> const&)>> playerChanged;

    static void LogIn(string login, string password, const function<void(optional<Player> const&, string)>& finished);
    static void SignUp(string login, string password, const function<void(optional<Player> const&, string)>& finished);
    static void LogOut();

    static optional<Player> currentPlayer;
    static string lastErrorDescription;
};
