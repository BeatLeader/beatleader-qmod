#pragma once

#include "EventCallback/invokable.hpp"
#include "main.hpp"
#include "UI/Abstract/AbstractReeModal.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "GlobalNamespace/FireworksController.hpp"

DECLARE_CLASS_CUSTOM(BeatLeader, PrestigePanelComponent, BeatLeader::ReeComponent) {
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _PrestigeYesButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, _PrestigeNoButton);
};

namespace BeatLeader {
    class PrestigePanel : public AbstractReeModal<PrestigePanelComponent*> {
        public:
            PrestigePanel() : uploadStateCallbackFunc(BeatLeader::UploadStateFunc{std::bind(&PrestigePanel::OnUploadStateChanged, this, std::placeholders::_1, std::placeholders::_2)}) {}
            using ComponentType = PrestigePanelComponent*;

            void OnInitialize() override;
            void OnDispose() override;
        
            StringW GetContent() override;
        private:
            BeatLeader::UploadStateCallbackFunc uploadStateCallbackFunc;
            GlobalNamespace::FireworksController* fireworksController;

            void OnProfileRequestStateChanged(Player player, ReplayUploadStatus state);
            void OnUploadStateChanged(std::optional<ScoreUpload> scoreUpload, ReplayUploadStatus state);
            void RequestPrestige();
            void Fireworks(double duration);
            void InitializePrestigeButtons();
    };
}

namespace BeatLeader::PrestigePanelStatic {
    using RequestStateChangedFunc = std::function<void()>;
    using RequestStateChangedCallbackFunc = callback<>;
    extern invokable<> RequestStateChanged;
}