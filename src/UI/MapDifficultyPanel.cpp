#include "include/UI/MapDifficultyPanel.hpp"

#include "include/UI/ModifiersUI.hpp"

#include "shared/Models/TriangleRating.hpp"

#include <cmath>

DEFINE_TYPE(BeatLeader, MapDifficultyPanelComponent);

namespace BeatLeader {

    void MapDifficultyPanelComponent::Awake() {
        _skillTriangle = SkillTriangle::Instantiate<SkillTriangle>(get_transform())->LocalComponent();
    }

    void MapDifficultyPanel::OnInitialize() {
        SetRootActive(false);

        if (_hasDifficulty) {
            RefreshVisuals();
        }
    }

    StringW MapDifficultyPanel::GetContent() {
        return StringW(R"(
            <vertical ignore-layout="true" id="root" pad-top="5" pref-width="42">
                <horizontal pref-height="30" bg="round-rect-panel">
                    <macro.as-host host="_skillTriangle">
                        <macro.reparent transform="_uiComponent"/>
                    </macro.as-host>
                </horizontal>
            </vertical>
        )");
    }

    void MapDifficultyPanel::Clear() {
        _difficulty = Difficulty();
        _hasDifficulty = false;
        _hoverEnabled = false;
        SetRootActive(false);
    }

    void MapDifficultyPanel::SetDifficulty(Difficulty const& difficulty) {
        _difficulty = difficulty;
        _hasDifficulty = true;
        RefreshVisuals();
    }

    void MapDifficultyPanel::RefreshVisuals() {
        if (!_hasDifficulty) {
            return;
        }

        auto rating = ModifiersUI::ApplyCurrentModifiers(_difficulty.rating);
        _hoverEnabled = (rating.techRating + rating.accRating + rating.passRating) > 0.0f;

        if (auto* skillTriangle = GetSkillTriangle()) {
            skillTriangle->SetValues(rating.techRating, rating.accRating, rating.passRating);
        }

        if (!_hoverEnabled) {
            SetRootActive(false);
        }
    }

    void MapDifficultyPanel::SetHoverState(UnityEngine::Vector3 const& worldPos, bool isHovered, float progress) {
        if (!_hoverEnabled || progress <= 0.3f || !LocalComponent()->root || !LocalComponent()->root->get_parent()) {
            SetRootActive(false);
            return;
        }

        SetRootActive(true);

        auto scale = std::pow(progress, isHovered ? 0.5f : 2.0f);
        LocalComponent()->root->set_localScale(UnityEngine::Vector3(0.5f + 0.5f * scale, scale, 1.0f));

        auto localPosition = LocalComponent()->root->get_parent()->InverseTransformPoint(worldPos);
        LocalComponent()->root->set_localPosition(UnityEngine::Vector3(localPosition.x, localPosition.y - 15.0f, 0.0f));
    }

    SkillTriangle* MapDifficultyPanel::GetSkillTriangle() {
        if (!LocalComponent() || !LocalComponent()->_skillTriangle) {
            return nullptr;
        }

        return reinterpret_cast<SkillTriangle*>(LocalComponent()->_skillTriangle->nativeComponent);
    }

    void MapDifficultyPanel::SetRootActive(bool value) {
        if (LocalComponent() && LocalComponent()->root) {
            LocalComponent()->root->get_gameObject()->SetActive(value);
        }
    }

}
