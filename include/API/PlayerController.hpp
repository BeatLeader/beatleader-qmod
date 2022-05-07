#include "Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
    static void RefreshPlatform();
    static string RefreshOnline();
public:
    static string Refresh();
    static vector<function<void(std::optional<Player> const&)>> playerChanged;

    static void LogIn(string_view login, string_view password, const std::function<void(std::string_view)>& finished);
    static void SignUp(string_view login, string_view password, const std::function<void(std::string_view)>& finished);
    static bool LogOut();

    static std::optional<Player> currentPlayer;
    static std::optional<Player> platformPlayer;
    static string lastErrorDescription;
};
