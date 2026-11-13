#include "include/UI/UploadDisablersUI.hpp"
#include "include/Utils/ModConfig.hpp"
#include "main.hpp"

#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/LevelParamsPanel.hpp"

#include "beatsaber-hook/shared/hooking.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "metacore-bs/shared/events.hpp"
#include "scotland2/shared/modloader.h"

#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"

using namespace GlobalNamespace;

namespace UploadDisablersUI {
    static safe_ptr<TMPro::TextMeshProUGUI*> label;
    static bool bslInstalled = false;
    static bool creationScheduled = false;

    static void Refresh() {
        if (!label) return;

        bool uploadEnabled = UploadEnabled();
        label->get_gameObject()->SetActive(!uploadEnabled);
        if (!uploadEnabled) {
            label->set_text(UploadDisablers());
        }
    }

    static void CreateLabel(StandardLevelDetailView* view) {
        auto text = BSML::Lite::CreateText(view->_levelParamsPanel->get_transform(), "", 3, UnityEngine::Vector2(-5, bslInstalled ? -22 : -19), UnityEngine::Vector2(60, 5));
        text->set_color(UnityEngine::Color(1.0, 0.0, 0.0, 1));
        text->set_alignment(TMPro::TextAlignmentOptions::Center);
        text->set_enableWordWrapping(false);
        BSML::Lite::AddHoverHint(text, "Check their settings for 'force' or 'hitbox'");
        label = text;
        Refresh();
    }

    MAKE_HOOK_MATCH(LevelRefreshContent, &StandardLevelDetailView::RefreshContent, void, StandardLevelDetailView* self) {
        LevelRefreshContent(self);

        if (!label && !creationScheduled && self->_beatmapLevel) {
            creationScheduled = true;
            safe_ptr<StandardLevelDetailView*> view = self;
            // A frame later, so BetterSongList has already cloned the panel with its children in its own RefreshContent hook
            BSML::MainThreadScheduler::ScheduleNextFrame([view] {
                creationScheduled = false;
                if (view && !label) CreateLabel(view.ptr());
            });
        }
        Refresh();
    }

    void setup() {
        INSTALL_HOOK(BeatLeaderLogger, LevelRefreshContent);
        MetaCore::Events::AddCallback(MetaCore::Events::ScoreSubmission, [] {
            BSML::MainThreadScheduler::Schedule(Refresh);
        });

        for (auto& modInfo : modloader::get_all()) {
            if (auto loadedMod = std::get_if<modloader::ModData>(&modInfo)) {
                if (loadedMod->info.id == "BetterSongList") {
                    bslInstalled = true;
                    break;
                }
            }
        }
    }
}
