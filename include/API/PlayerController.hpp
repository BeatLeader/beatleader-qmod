#pragma once

#include "shared/Models/Player.hpp"

#include <string>
using namespace std;

class PlayerController
{
public:
    static void Refresh(int retry = 0, const function<void(optional<Player> const&, string)>& finished = nullptr);
    static vector<function<void(optional<Player> const&)>> playerChanged;

    static void LogIn(string login, string password, const function<void(optional<Player> const&, string)>& finished);
    static void SignUp(string login, string password, const function<void(optional<Player> const&, string)>& finished);
    static void LogOut();

    static bool IsFriend(Player anotherPlayer);
    static bool IsPatron(Player anotherPlayer);

    static bool IsIncognito(Player anotherPlayer);
    static void SetIsIncognito(Player anotherPlayer, bool value);

    static bool InClan(string tag);
    static bool IsMainClan(string tag);

    static optional<Player> currentPlayer;
    static string lastErrorDescription;
};
