#pragma once

#include "UI/ReeUIComponentV2.hpp"
#include "UI/SkillTriangle.hpp"
#include "shared/Models/Difficulty.hpp"

#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector3.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, MapDifficultyPanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, root);
    DECLARE_INSTANCE_FIELD(BeatLeader::SkillTriangleComponent*, _skillTriangle);

    DECLARE_INSTANCE_METHOD(void, Awake);
};

namespace BeatLeader {

class MapDifficultyPanel : public ReeUIComponentV2<MapDifficultyPanelComponent*> {
public:
    void OnInitialize() override;
    StringW GetContent() override;

    void Clear();
    void SetDifficulty(Difficulty const& difficulty);
    void RefreshVisuals();
    void SetHoverState(UnityEngine::Vector3 const& worldPos, bool isHovered, float progress);

private:
    SkillTriangle* GetSkillTriangle();
    void SetRootActive(bool value);

    Difficulty _difficulty {};
    bool _hasDifficulty = false;
    bool _hoverEnabled = false;
};

}
