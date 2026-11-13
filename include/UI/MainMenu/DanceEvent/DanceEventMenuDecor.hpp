#pragma once

#include "custom-types/shared/macros.hpp"

#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, DanceEventMenuDecor, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Start);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

public:
    static void setup();
};
