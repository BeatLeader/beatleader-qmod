#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "sombrero/shared/FastVector2.hpp"

#include "custom-types/shared/macros.hpp"

using namespace std;

#define GET_FIND_METHOD(mPtr) il2cpp_utils::il2cpp_type_check::MetadataGetter<mPtr>::get()

DECLARE_CLASS_CODEGEN(BeatLeader, AccuracyGraphLine, UnityEngine::UI::Graphic,
    DECLARE_INSTANCE_FIELD(UnityEngine::Rect, viewRect);
    DECLARE_INSTANCE_FIELD(float, canvasRadius);

    DECLARE_INSTANCE_METHOD(void, Setup, ArrayWrapper<Sombrero::FastVector2> points, UnityEngine::Rect viewRect, float canvasRadius);
    DECLARE_OVERRIDE_METHOD(void, OnPopulateMesh, GET_FIND_METHOD(static_cast<void (UnityEngine::UI::Graphic::*)(UnityEngine::UI::VertexHelper*)>(&UnityEngine::UI::Graphic::OnPopulateMesh)), UnityEngine::UI::VertexHelper* vh);
)