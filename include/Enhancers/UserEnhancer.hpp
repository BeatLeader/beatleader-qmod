#include "include/Models/Replay.hpp"

class UserEnhancer
{
private:
    void EnhanceLocal(Replay* replay);
    void EnhanceBL(Replay* replay);
public:
    UserEnhancer(/* args */);
    ~UserEnhancer();

    void Enhance(Replay* replay);
};
