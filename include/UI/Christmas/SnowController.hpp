#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/ParticleSystem.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, SnowController, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::ParticleSystem*, _particleSystem);

    DECLARE_INSTANCE_METHOD(void, OnTransformParentChanged);
    DECLARE_INSTANCE_METHOD(void, Play, bool immediately);
    DECLARE_INSTANCE_METHOD(void, Stop);
) 