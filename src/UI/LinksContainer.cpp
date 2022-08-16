#include "include/UI/LinksContainer.hpp"

#include "include/Assets/BundleLoader.hpp"
#include "include/Utils/WebUtils.hpp"
#include "include/Utils/PlaylistSynchronizer.hpp"
#include "include/API/PlayerController.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Application.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/Component.hpp"

#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "main.hpp"

using namespace QuestUI::BeatSaberUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;
using namespace std;

void BeatLeader::initLinksContainerPopup(BeatLeader::LinksContainerPopup** modalUIPointer, Transform* parent){

    auto modalUI = *modalUIPointer;
    if (modalUI != nullptr){
        UnityEngine::GameObject::Destroy(modalUI->modal->get_gameObject());
    }
    if (modalUI == nullptr) modalUI = (BeatLeader::LinksContainerPopup*) malloc(sizeof(BeatLeader::LinksContainerPopup));

    auto container = CreateModal(parent, UnityEngine::Vector2(75, 50), [](HMUI::ModalView *modal) {}, true);
    modalUI->modal = container;

    auto modalTransform = container->get_transform();

    modalUI->versionText = CreateText(modalTransform, "Loading...", UnityEngine::Vector2(-4.0, 14.0));
    CreateText(modalTransform, "<u>These buttons will open the browser!", UnityEngine::Vector2(-4.0, 4.0));

    modalUI->profile = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->websiteLinkIcon, UnityEngine::Vector2(-24, -1), UnityEngine::Vector2(22, 6), [](){
        string url = WebUtils::WEB_URL;
        if (PlayerController::currentPlayer != std::nullopt) {
            url += "u/" + PlayerController::currentPlayer->id;
        }
        UnityEngine::Application::OpenURL(url);
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->profile, "Your web profile");

    modalUI->discord = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->discordLinkIcon, UnityEngine::Vector2(0, -1), UnityEngine::Vector2(22, 6), [](){
        UnityEngine::Application::OpenURL("https://discord.gg/2RG5YVqtG6");
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->discord, "Our discord server");

    modalUI->patreon = ::QuestUI::BeatSaberUI::CreateClickableImage(modalTransform, BundleLoader::bundle->patreonLinkIcon, UnityEngine::Vector2(24, -1), UnityEngine::Vector2(22, 6), [](){
        UnityEngine::Application::OpenURL("https://patreon.com/BeatLeader");
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->patreon, "Patreon page");

    WebUtils::GetJSONAsync(WebUtils::API_URL + "mod/lastVersions", [modalUI](long status, bool error, rapidjson::Document const& result){ 
        if (status == 200 && !error && result.HasMember("quest")) {
            string version = result["quest"].GetObject()["version"].GetString();
            QuestUI::MainThreadScheduler::Schedule([modalUI, version] {
                if (modInfo.version == version) {
                    modalUI->versionText->SetText("<color=#88FF88>Mod is up to date!");
                } else  {
                    modalUI->versionText->SetText("<color=#FF8888>Mod is outdated!");
                }
            });
        }
    });

    CreateText(modalTransform, "<u>Install playlists. You need to sync them yourself!", UnityEngine::Vector2(-4.0, -11.0));
    modalUI->nominated = ::QuestUI::BeatSaberUI::CreateUIButton(modalTransform, "Nominated", UnityEngine::Vector2(-24.0, -19.0), [modalUI]() {
       PlaylistSynchronizer::InstallPlaylist(WebUtils::API_URL + "playlist/nominated", "BL Nominated");
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->nominated, "Playlist of nominated maps");

    modalUI->qualified = ::QuestUI::BeatSaberUI::CreateUIButton(modalTransform, "Qualified", UnityEngine::Vector2(0, -19.0), [modalUI]() {
        PlaylistSynchronizer::InstallPlaylist(WebUtils::API_URL + "playlist/qualified", "BL Qualified");
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->nominated, "Playlist of qualified maps");

    modalUI->ranked = ::QuestUI::BeatSaberUI::CreateUIButton(modalTransform, "Ranked", UnityEngine::Vector2(24.0, -19.0), [modalUI]() {
        PlaylistSynchronizer::InstallPlaylist(WebUtils::API_URL + "playlist/ranked", "BL Ranked");
    });
    ::QuestUI::BeatSaberUI::AddHoverHint(modalUI->nominated, "Playlist of ranked maps");

    modalUI->modal->set_name("BLLinksModal");
    *modalUIPointer = modalUI;
}