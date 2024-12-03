// #include "UI/BSML_Addons/BSMLAddonsLoader.hpp"
// #include "UI/BSML_Addons/Tags/BetterButtonTag.hpp"
// #include "UI/BSML_Addons/Tags/BetterImageTag.hpp"
// #include "Assets/BundleLoader.hpp"
// #include "Utils/BSMLUtility.hpp"

// namespace BeatLeader::UI::BSML_Addons {
//     bool BSMLAddonsLoader::ready = false;

//     std::unordered_map<std::string, UnityEngine::Sprite*> BSMLAddonsLoader::spritesToCache = {
//         {"black-transparent-bg", BundleLoader::bundle->BlackTransparentBG},
//         {"black-transparent-bg-outline", BundleLoader::bundle->BlackTransparentBGOutline},
//         {"cyan-bg-outline", BundleLoader::bundle->CyanBGOutline},
//         {"white-bg", BundleLoader::bundle->WhiteBG},
//         {"closed-door-icon", BundleLoader::bundle->ClosedDoorIcon},
//         {"opened-door-icon", BundleLoader::bundle->OpenedDoorIcon},
//         {"edit-layout-icon", BundleLoader::bundle->EditLayoutIcon},
//         {"settings-icon", BundleLoader::bundle->settingsIcon},
//         {"replayer-settings-icon", BundleLoader::bundle->ReplayerSettingsIcon},
//         {"left-arrow-icon", BundleLoader::bundle->LeftArrowIcon},
//         {"right-arrow-icon", BundleLoader::bundle->RightArrowIcon},
//         {"play-icon", BundleLoader::bundle->PlayIcon},
//         {"pause-icon", BundleLoader::bundle->PauseIcon},
//         {"lock-icon", BundleLoader::bundle->LockIcon},
//         {"warning-icon", BundleLoader::bundle->WarningIcon},
//         {"cross-icon", BundleLoader::bundle->CrossIcon},
//         {"pin-icon", BundleLoader::bundle->PinIcon},
//         {"align-icon", BundleLoader::bundle->AlignIcon},
//         {"anchor-icon", BundleLoader::bundle->AnchorIcon},
//         {"progress-ring-icon", BundleLoader::bundle->ProgressRingIcon},
//         {"refresh-icon", BundleLoader::bundle->RotateRightIcon}
//     };

//     void BSMLAddonsLoader::LoadAddons() {
//         if (!ready) {
//             for (const auto& [key, sprite] : spritesToCache) {
//                 if (sprite) {
//                     Utils::BSMLUtility::AddSpriteToBSMLCache("bl-" + key, sprite);
//                 }
//             }
//         }
        
//         ready = true;
//     }
// } 