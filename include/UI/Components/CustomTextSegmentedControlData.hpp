#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/GameObject.hpp"

#include "HMUI/SegmentedControl.hpp"
#include "HMUI/SegmentedControlCell.hpp"
#include "HMUI/TextSegmentedControlCell.hpp"

#include <vector>
#include <string>

DECLARE_CLASS_CODEGEN_INTERFACES(QuestUI, CustomTextSegmentedControlData, UnityEngine::MonoBehaviour, HMUI::SegmentedControl::IDataSource*) {
    DECLARE_INSTANCE_FIELD(HMUI::SegmentedControl*, segmentedControl);

    DECLARE_INSTANCE_FIELD(HMUI::SegmentedControlCell*, firstCellPrefab);
    DECLARE_INSTANCE_FIELD(HMUI::SegmentedControlCell*, lastCellPrefab);
    DECLARE_INSTANCE_FIELD(HMUI::SegmentedControlCell*, singleCellPrefab);
    DECLARE_INSTANCE_FIELD(HMUI::SegmentedControlCell*, middleCellPrefab);
    DECLARE_INSTANCE_FIELD(ArrayW<StringW>, texts);

    DECLARE_OVERRIDE_METHOD(int, NumberOfCells, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::SegmentedControl::IDataSource::NumberOfCells>::methodInfo());
    DECLARE_OVERRIDE_METHOD(HMUI::SegmentedControlCell*, CellForCellNumber, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::SegmentedControl::IDataSource::CellForCellNumber>::methodInfo(), int idx);
    DECLARE_CTOR(ctor);
    DECLARE_DTOR(dtor);
    public:
        void set_texts(ArrayW<StringW> list);
        void add_text(StringW addedText);

        float fontSize = 3.0f;
        bool hideCellBackground = false;
        bool overrideCellSize = true;
        float padding = 2.0f;
    private:
        HMUI::TextSegmentedControlCell* InstantiateCell(UnityEngine::GameObject* prefab);
};