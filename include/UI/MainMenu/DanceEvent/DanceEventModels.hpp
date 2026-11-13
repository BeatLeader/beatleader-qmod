#pragma once

#include "UnityEngine/Color.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Mesh.hpp"

#include "beatsaber-hook/shared/safeptr.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace DanceEventModels {
    struct Model {
        safe_ptr<UnityEngine::Mesh*> mesh;
        std::vector<std::string> materials;
    };

    Model const* Get(std::string const& file);

    UnityEngine::Material* Solid(UnityEngine::Color color, float minimumLight);
    UnityEngine::Material* Glowing(UnityEngine::Color color);

    UnityEngine::Color Hex(std::string_view hex);
}
