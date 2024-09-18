#include "include/UI/MultiplayerLeaderboard.hpp"

#include "GlobalNamespace/MainFlowCoordinator.hpp"
#include "GlobalNamespace/MultiplayerLobbyController.hpp"
#include "GlobalNamespace/ServerPlayerListViewController.hpp"
#include "GlobalNamespace/PlatformLeaderboardViewController.hpp"
#include "GlobalNamespace/LevelSelectionNavigationController.hpp"
#include "GlobalNamespace/StandardLevelDetailViewController.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/FlowCoordinator.hpp"

#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"

#include "custom-types/shared/register.hpp"
#include "custom-types/shared/delegate.hpp"

#include "main.hpp"

using namespace GlobalNamespace;
using namespace HMUI;

namespace MultiplayerLeaderboardManager {
    MainFlowCoordinator* _mainFlowCoordinator;
    MultiplayerLobbyController* _multiplayerLobbyController;
    ServerPlayerListViewController* _serverPlayerListViewController;
    PlatformLeaderboardViewController* _platformLeaderboardViewController;
    LevelSelectionNavigationController* _levelSelectionNavigationController;

    ::System::Action_1<::UnityW<LevelSelectionNavigationController>>* delegate1;
    ::System::Action_2<UnityW<LevelSelectionNavigationController>, StandardLevelDetailViewController::ContentType>* delegate2;

    void HideLeaderboard() {
        _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf()->SetRightScreenViewController(nullptr, ViewController::AnimationType::Out);
    }

    void ShowLeaderboard(BeatmapLevel* difficultyBeatmap) {
        if (!_multiplayerLobbyController->lobbyActivated)
            return;

        if (difficultyBeatmap == nullptr) {
            HideLeaderboard();
            return;
        }

        auto beatmapKey = _levelSelectionNavigationController->beatmapKey;
        _platformLeaderboardViewController->SetData(ByRef<GlobalNamespace::BeatmapKey>(&beatmapKey));

        _mainFlowCoordinator->YoungestChildFlowCoordinatorOrSelf()->SetRightScreenViewController(_platformLeaderboardViewController, ViewController::AnimationType::In);
        _serverPlayerListViewController->DeactivateGameObject();
    }

    void LevelSelectionNavigationControllerOndidChangeLevelDetailContentEvent(LevelSelectionNavigationController* levelSelectionNavigationController, StandardLevelDetailViewController::ContentType contentType) {
        if (contentType == StandardLevelDetailViewController::ContentType::OwnedAndReady) {
            ShowLeaderboard(levelSelectionNavigationController->beatmapLevel);
            return;
        }

        ShowLeaderboard(nullptr);
    }

    void LevelSelectionNavigationControllerOndidChangeDifficultyBeatmapEvent(LevelSelectionNavigationController* levelSelectionNavigationController) {
        ShowLeaderboard(levelSelectionNavigationController->beatmapLevel);
    }

    void Initialize(Zenject::DiContainer* container) {
        _mainFlowCoordinator = reinterpret_cast<MainFlowCoordinator*>(container->Resolve(csTypeOf(MainFlowCoordinator*)));
        _multiplayerLobbyController = reinterpret_cast<MultiplayerLobbyController*>(container->Resolve(csTypeOf(MultiplayerLobbyController*)));
        _serverPlayerListViewController = reinterpret_cast<ServerPlayerListViewController*>(container->Resolve(csTypeOf(ServerPlayerListViewController*)));
        _platformLeaderboardViewController = reinterpret_cast<PlatformLeaderboardViewController*>(container->Resolve(csTypeOf(PlatformLeaderboardViewController*)));
        _levelSelectionNavigationController = reinterpret_cast<LevelSelectionNavigationController*>(container->Resolve(csTypeOf(LevelSelectionNavigationController*)));

        delegate1 = custom_types::MakeDelegate<System::Action_1<UnityW<LevelSelectionNavigationController>> *>((std::function<void(UnityW<LevelSelectionNavigationController>)>)LevelSelectionNavigationControllerOndidChangeDifficultyBeatmapEvent);
        delegate2 = custom_types::MakeDelegate<System::Action_2<UnityW<LevelSelectionNavigationController>, StandardLevelDetailViewController::ContentType> *>((std::function<void(UnityW<LevelSelectionNavigationController>, StandardLevelDetailViewController::ContentType)>)LevelSelectionNavigationControllerOndidChangeLevelDetailContentEvent);
        _levelSelectionNavigationController->add_didChangeDifficultyBeatmapEvent(delegate1);
        _levelSelectionNavigationController->add_didChangeLevelDetailContentEvent(delegate2);
    }

    void Dispose() {
        _levelSelectionNavigationController->remove_didChangeDifficultyBeatmapEvent(delegate1);
        _levelSelectionNavigationController->remove_didChangeLevelDetailContentEvent(delegate2);
    }
}
