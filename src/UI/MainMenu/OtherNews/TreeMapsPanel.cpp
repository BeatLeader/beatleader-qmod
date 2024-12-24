#include "UI/MainMenu/OtherNews/TreeMapsPanel.hpp"
#include "UI/MainMenu/Modals/MapDownloadDialog.hpp"
#include "Assets/BundleLoader.hpp"
#include "Assets/Sprites.hpp"
#include "Utils/WebUtils.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "Utils/FormatUtils.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, TreeMapsPanelComponent);

namespace BeatLeader {

    void TreeMapsPanelComponent::Awake() {
        header = NewsHeader::Instantiate<NewsHeader>(get_transform())->LocalComponent();
    }

    void TreeMapsPanel::OnInitialize() {
        LocalComponent()->header->SetupData("Project Tree");
        LocalComponent()->header->_image->set_color(UnityEngine::Color(0.027f, 0.43f, 0.0f, 1.0f));

        // LocalComponent()->_songCover->set_material(BundleLoader::bundle->roundTexture10Material);
        // LocalComponent()->_songCover->__Refresh();

        // LocalComponent()->_topText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);
        // LocalComponent()->_bottomText->set_overflowMode(TMPro::TextOverflowModes::Ellipsis);

        // LocalComponent()->button->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(
        //     (std::function<void()>)[this] { 
        //         OnPlayButtonPressed();
        //     }
        // ));
    }

    // void TreeMapsPanel::OnRootStateChange(bool active) {
    //     BeatLeaderLogger.info("TreeMapsPanel::OnRootStateChange 1");
    //     if (active) {
    //         // API::RequestManager::SendTreeStatusRequest();
    //         // API::RequestManager::AddTreeStatusRequestListener([this](API::RequestState state, TreeStatus* status, StringW failReason) {
    //         //     HandleTreeRequestState(state, status, failReason);
    //         // });
    //     } else {
    //         // API::RequestManager::RemoveTreeStatusRequestListener(
    //         //     std::bind(&TreeMapsPanel::HandleTreeRequestState, this,
    //         //               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //     }
    //     BeatLeaderLogger.info("TreeMapsPanel::OnRootStateChange 2");
    // }

    // void TreeMapsPanel::OnDispose() {
    //     // API::RequestManager::RemoveTreeStatusRequestListener(
    //     //     std::bind(&TreeMapsPanel::HandleTreeRequestState, this,
    //     //               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // }

    // void TreeMapsPanel::HandleTreeRequestState(API::RequestState state, TreeStatus* result, StringW failReason) {
    //     BeatLeaderLogger.info("TreeMapsPanelComponent::HandleTreeRequestState 1");
    //     switch (state) {
    //         case API::RequestState::Started:
    //             LocalComponent()->loadingIndicator->SetActive(true);
    //             LocalComponent()->detailsContainer->SetActive(false);
    //             break;

    //         case API::RequestState::Failed:
    //             LocalComponent()->loadingIndicator->SetActive(false);
    //             // if (shouldShowOtherMapsEvent) {
    //             //     shouldShowOtherMapsEvent(true);
    //             // }
    //             break;

    //         case API::RequestState::Finished:
    //             LocalComponent()->loadingIndicator->SetActive(false);
    //             LocalComponent()->detailsContainer->SetActive(true);
    //             if (result->today->score) {
    //                 // if (shouldShowOtherMapsEvent) {
    //                 //     shouldShowOtherMapsEvent(true);
    //                 // }
    //             } else {
    //                 // currentStatus = result;
    //                 // UpdateUI(currentStatus);
    //             }
    //             break;
    //     }
    //     BeatLeaderLogger.info("TreeMapsPanelComponent::HandleTreeRequestState 2");
    // }

    // void TreeMapsPanel::UpdateUI(TreeStatus* status) {
    //     int ninetenthDay = 1734566400;
    //     int day = ninetenthDay - status->today->startTime;

    //     LocalComponent()->dateText->set_text("December " + std::to_string(19 + (int)(day / 86400)));

    //     LocalComponent()->topText->set_text(status->today->song.name);
    //     LocalComponent()->bottomText->set_text(status->today->song.mapper);

    //     Sprites::get_Icon(status->today->song.coverImage, [this](UnityEngine::Sprite* sprite) {
    //         LocalComponent()->image->set_sprite(sprite);
    //     });

    //     Sprites::get_Icon("https://cdn.assets.beatleader.xyz/project_tree_ornament_" + std::to_string(status->today->bundleId) + "_preview.png", [this](UnityEngine::Sprite* sprite) {
    //         LocalComponent()->ornamentTease->set_sprite(sprite);
    //     });

    //     auto bottomText = FormatUtils::GetRelativeTimeString(std::to_string(status->today->startTime + 60 * 60 * 24));
    //     LocalComponent()->timeText->set_text(bottomText);
    // }

    void TreeMapsPanel::OnPlayButtonPressed() {
        // if (currentStatus) {
        //     MapDownloadDialog::OpenSongOrDownloadDialog(currentStatus->today->song, LocalComponent()->get_transform());
        // }
    }

    StringW TreeMapsPanel::GetContent() {
        return StringW(R"(
            <vertical id="background" pad="1" spacing="1" vertical-fit="PreferredSize" pref-width="80" bg="round-rect-panel" bg-color="#00ff0055">
                <macro.as-host host="header">
                    <macro.reparent transform="_uiComponent"/>
                </macro.as-host>
            </vertical>
        )");
    }

} // namespace BeatLeader 