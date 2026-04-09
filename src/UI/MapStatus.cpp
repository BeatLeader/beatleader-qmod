#include "include/UI/MapStatus.hpp"

#include "include/UI/Components/SmoothHoverController.hpp"
#include "include/UI/ModifiersUI.hpp"
#include "include/Utils/FormatUtils.hpp"

#include "TMPro/TextAlignmentOptions.hpp"

#include <cmath>
#include <utility>

DEFINE_TYPE(BeatLeader, MapStatusComponent);

namespace BeatLeader {

    void MapStatus::OnInitialize() {
        LocalComponent()->background->set_raycastTarget(true);
        auto color = UnityEngine::Color(1.0f, 1.0f, 1.0f, 221.0f / 255.0f);
        LocalComponent()->background->set_color(color);
        LocalComponent()->background->set_color0(color);
        LocalComponent()->background->set_color1(color);
        LocalComponent()->statusText->set_alignment(TMPro::TextAlignmentOptions::Midline);
        LocalComponent()->statusText->set_richText(true);
        LocalComponent()->statusText->set_enableWordWrapping(false);

        SmoothHoverController::Custom(LocalComponent()->background->get_gameObject(), [this](bool hovered, float progress) {
            if (!_hoverCallback) {
                return;
            }

            _hoverCallback(LocalComponent()->background->get_transform()->get_position(), hovered, progress);
        });

        if (_hasDifficulty) {
            UpdateVisuals();
        }
    }

    StringW MapStatus::GetContent() {
        return StringW(R"(
            <horizontal id="background" pref-height="6" pad-left="1" pad-right="1" horizontal-fit="PreferredSize" bg="round-rect-panel">
                <text id="statusText" align="Midline" word-wrapping="false"/>
            </horizontal>
        )");
    }

    void MapStatus::SetActive(bool value) {
        LocalComponent()->SetRootActive(value);

        if (!value && _hoverCallback) {
            _hoverCallback(LocalComponent()->background->get_transform()->get_position(), false, 0.0f);
        }
    }

    void MapStatus::SetValues(int rankedStatus, Difficulty const& difficulty) {
        _rankedStatus = rankedStatus;
        _difficulty = difficulty;
        _hasDifficulty = true;
        UpdateVisuals();
    }

    void MapStatus::RefreshVisuals() {
        if (!_hasDifficulty) {
            return;
        }

        UpdateVisuals();
    }

    void MapStatus::SetHoverCallback(std::function<void(UnityEngine::Vector3 const&, bool, float)> callback) {
        _hoverCallback = std::move(callback);
    }

    void MapStatus::UpdateVisuals() {
        auto displayedRating = ModifiersUI::ApplyCurrentModifiers(_difficulty.rating);
        auto stars = displayedRating.stars;
        auto modifiersApplied = std::abs(stars - _difficulty.rating.stars) > 1e-4f;

        auto text = GetRankedStatusText(_rankedStatus);
        if (_difficulty.rating.stars > 0.0f) {
            text += ": " + FormatUtils::FormatStars(stars);
            if (modifiersApplied) {
                text += " <color=green>[M]</color>";
            }
        }

        LocalComponent()->statusText->SetText(text, true);
    }

    std::string MapStatus::GetRankedStatusText(int rankedStatus) {
        switch (rankedStatus) {
            case 0: return "Unranked";
            case 1: return "Nominated";
            case 2: return "Qualified";
            case 3: return "Ranked";
            case 4: return "Unrankable";
            case 5: return "Outdated";
            case 6: return "Event";
            case 7: return "OST";
            default: return "Unknown";
        }
    }

}
