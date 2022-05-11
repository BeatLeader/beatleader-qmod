#include <unordered_map>
#include <string>
using namespace std;

class ModifiersManager
{
public:
    static unordered_map<string, float> modifiers;
    static void Sync();
};
