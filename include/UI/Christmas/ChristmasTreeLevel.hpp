#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Gizmos.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Mathf.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, ChristmasTreeLevel, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(float, topRadius);
    DECLARE_INSTANCE_FIELD(float, topHeight);
    DECLARE_INSTANCE_FIELD(float, bottomRadius);
    DECLARE_INSTANCE_FIELD(float, bottomHeight);

    DECLARE_INSTANCE_METHOD(void, Draw, float factor);
    private:
        static void DrawCircle(UnityEngine::Vector3 center, float radius, int segments);
) 