#include <map>
#include "UnityEngine/Sprite.hpp"

namespace BeatLeader {
    class PrestigeLevelIconsManager {
        public:
            void Init();
            UnityEngine::Sprite* getSprite(int level);
        private:
            std::map<int, std::vector<uint8_t>> loadedSprites;
    };

    namespace PrestigeLevelIconsManagerNS {
        extern PrestigeLevelIconsManager Instance;
    }
}