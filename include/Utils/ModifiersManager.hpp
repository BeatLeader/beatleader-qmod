#include <map>
#include <string>
using namespace std;

class ModifiersManager
{
public:
    static map<string, float> modifiers;
    static void Sync();
};
