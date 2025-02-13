#include "shared/Models/Replay.hpp"

class UserEnhancer
{
private:
    static std::string hmd;
    void EnhanceLocal(Replay& replay);
    void EnhanceBL(Replay& replay);
public:
    static void FetchHMD();
    void Enhance(Replay& replay);
};
