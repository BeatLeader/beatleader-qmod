#include "include/Models/Replay.hpp"

class UserEnhancer
{
private:
    void EnhanceLocal(Replay& replay);
    void EnhanceBL(Replay& replay);
public:
    void Enhance(Replay& replay);
};
