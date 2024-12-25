#pragma once

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Christmas/ChristmasTreeEditorPanel.hpp"
#include "UI/Christmas/OrnamentStorePanel.hpp"
#include "UI/Christmas/BonusOrnamentStorePanel.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, ChristmasTreeEditor, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(bool, isOpened);
    DECLARE_INSTANCE_FIELD(ChristmasTree*, tree);
    DECLARE_INSTANCE_FIELD(ChristmasTreeEditorPanelComponent*, editorPanel);
    DECLARE_INSTANCE_FIELD(OrnamentStorePanel*, ornamentStore);
    DECLARE_INSTANCE_FIELD(BonusOrnamentStorePanel*, bonusOrnamentStore);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, editorPos);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, initialPos);
    DECLARE_INSTANCE_FIELD(float, editorScale);
    DECLARE_INSTANCE_FIELD(float, initialScale);
    DECLARE_INSTANCE_FIELD(bool, uploading);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, SetupTree, ChristmasTree* tree);
    DECLARE_INSTANCE_METHOD(void, Present);
    DECLARE_INSTANCE_METHOD(void, Dismiss);
    DECLARE_INSTANCE_METHOD(void, SetUploading, bool loading);
    
    private:
        void OnTreeRequestStateChanged(API::RequestState state, StringW result, StringW failReason);
        void UploadSettings();

    // public:
    //     custom_types::Delegate<void> EditorClosedEvent;
)