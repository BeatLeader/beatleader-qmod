#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/HoverHint.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"
#include "Models/TreeStatus.hpp"
#include "Models/MapDetail.hpp"
#include "UI/Christmas/ChristmasTreeOrnament.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, OrnamentStoreCell, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(ChristmasTreeOrnament*, previewInstance);
    DECLARE_INSTANCE_FIELD(int, bundleId);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, text);
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, previewImage);
    DECLARE_INSTANCE_FIELD(HMUI::HoverHint*, hint);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, SetOpeningDayIndex, int dayIndex);
    
    DECLARE_INSTANCE_METHOD(void, ReloadNextInstance, bool despawn);
    DECLARE_INSTANCE_METHOD(void, HandlePreviewOrnamentGrabbed, ChristmasTreeOrnament* ornament);

    public:
        MapDetail* song;
        void SetOrnamentStatus(std::optional<DailyTreeStatus> status);
        void SetBonusOrnamentStatus(std::optional<BonusOrnament> status);

    private:
        static constexpr float CELL_SIZE = 10.0f;
)