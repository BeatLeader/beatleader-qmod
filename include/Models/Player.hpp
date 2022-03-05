#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include <string>
using namespace std;

class Player
{
private:
    /* data */
public:
    string id;
    string name;
    string country;
    string avatar;
    int rank;
    int countryRank;
    float pp;

    Player();
    Player(rapidjson::Document& document);
    ~Player();
};
