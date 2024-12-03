#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "bsml/shared/BSML.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Vector2.hpp"

#include "UI/MainMenu/TextNews/TextNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/MapNewsPanel.hpp"
#include "UI/MainMenu/OtherNews/EventNewsPanel.hpp"

DECLARE_CLASS_CODEGEN(BeatLeader, BeatLeaderNewsViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(BeatLeader::AbstractNewsPanelComponent*, textNewsPanel);
    DECLARE_INSTANCE_FIELD(BeatLeader::AbstractNewsPanelComponent*, mapNewsPanel);
    DECLARE_INSTANCE_FIELD(BeatLeader::AbstractNewsPanelComponent*, eventNewsPanel);
    
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, screenTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector2, originalScreenSize);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector2, originalPosition);
    DECLARE_INSTANCE_FIELD(bool, initialized);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHeirarchy, bool screenSystemDisabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidDeactivate, &HMUI::ViewController::DidDeactivate, bool removedFromHierarchy, bool screenSystemDisabling);

    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

    DECLARE_INSTANCE_METHOD(bool, LazyInitializeScreen);
    DECLARE_INSTANCE_METHOD(void, UpdateScreen);
    DECLARE_INSTANCE_METHOD(void, RevertScreenChanges);
    
    public:
        static UnityEngine::Vector2 GetTargetScreenSize();
        static constexpr const char* get_content() {
            return R"(
                <bg id='root-object'
                    anchor-pos-x='20'
                    xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'
                    xsi:schemaLocation='https://monkeymanboy.github.io/BSML-Docs/ https://monkeymanboy.github.io/BSML-Docs/BSMLSchema.xsd'>
                    <horizontal spacing="1" pref-width='180'>
                        <vertical spacing="0" vertical-fit="PreferredSize">
                            <macro.as-host host="textNewsPanel">
                                <macro.reparent transform="_uiComponent"/>
                            </macro.as-host>
                        </vertical>
                        <vertical spacing="1" vertical-fit="PreferredSize">
                            <macro.as-host host="eventNewsPanel">
                                <macro.reparent transform="_uiComponent"/>
                            </macro.as-host>
                            <macro.as-host host="mapNewsPanel">
                                <macro.reparent transform="_uiComponent"/>
                            </macro.as-host>
                        </vertical>
                    </horizontal>
                </bg>
            )";
        }
        
) 

