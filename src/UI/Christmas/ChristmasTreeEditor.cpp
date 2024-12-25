#include "UI/Christmas/ChristmasTreeEditor.hpp"
#include "API/RequestManager.hpp"
#include "Utils/UnityExtensions.hpp"
#include "custom-types/shared/delegate.hpp"

#include "main.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeEditor);

namespace BeatLeader {

    void ChristmasTreeEditor::Awake() {
        editorPos = UnityEngine::Vector3(0.0f, 0.0f, 1.0f);
        editorScale = 1.0f;
        isOpened = false;

        BeatLeaderLogger.info("ChristmasTreeEditor::Awake 1");

        // Create OrnamentStore
        auto ornamentStoreObj = UnityEngine::GameObject::New_ctor("OrnamentStorePanel");
        ornamentStore = ornamentStoreObj->AddComponent<OrnamentStorePanel*>();
        auto trans = ornamentStoreObj->get_transform();
        trans->SetParent(get_transform(), false);
        trans->set_localPosition(UnityEngine::Vector3(-0.8f, 1.0f, 0.3f));
        trans->set_localEulerAngles(UnityEngine::Vector3(0.0f, 300.0f, 0.0f));
        BeatLeaderLogger.info("ChristmasTreeEditor::Awake 2");

        // Create BonusOrnamentStore
        auto bonusStoreObj = UnityEngine::GameObject::New_ctor("BonusOrnamentStorePanel");
        bonusOrnamentStore = bonusStoreObj->AddComponent<BonusOrnamentStorePanel*>();
        trans = bonusStoreObj->get_transform();
        trans->SetParent(get_transform(), false);
        trans->set_localPosition(UnityEngine::Vector3(-1.1f, 1.0f, -0.25f));
        trans->set_localEulerAngles(UnityEngine::Vector3(0.0f, 280.0f, 0.0f));
        BeatLeaderLogger.info("ChristmasTreeEditor::Awake 3");

        // Create EditorPanel
        auto editorPanelObj = ChristmasTreeEditorPanel::Instantiate<ChristmasTreeEditorPanel>(get_transform());
        editorPanelObj->LocalComponent()->ManualInit(get_transform());
        editorPanel = editorPanelObj->LocalComponent();
        
        trans = editorPanel->GetRootTransform();
        trans->set_localPosition(UnityEngine::Vector3(0.8f, 1.0f, 0.3f));
        trans->set_localEulerAngles(UnityEngine::Vector3(0.0f, 60.0f, 0.0f));
        editorPanel->cancelAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
            Dismiss();
            API::RequestManager::SendChristmasTreeRequest();
        });
        editorPanel->saveAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
            UploadSettings();
        });
        editorPanel->clearAction = custom_types::MakeDelegate<System::Action*>((std::function<void()>)[this]() {
            tree->ClearOrnaments();
        });
        BeatLeaderLogger.info("ChristmasTreeEditor::Awake 4");
    }

    void ChristmasTreeEditor::SetupTree(ChristmasTree* treeInstance) {
        tree = treeInstance;
        editorPanel->SetupTree(tree);
    }

    void ChristmasTreeEditor::Present() {
        isOpened = true;
        initialPos = tree->get_transform()->get_position();
        initialScale = tree->get_transform()->get_localScale().y;
        
        tree->ScaleTo(editorScale, false);
        tree->MoveTo(editorPos, false);
        tree->SetMoverRestricted(true);
        tree->SetOrnamentsMovement(true);
        
        ornamentStore->Present();
        bonusOrnamentStore->Present();
        
        editorPanel->Present();

        API::RequestManager::AddTreeOrnamentsRequestListener(
            std::bind(&ChristmasTreeEditor::OnTreeRequestStateChanged, this, 
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void ChristmasTreeEditor::Dismiss() {
        isOpened = false;
        tree->MoveTo(initialPos, false);
        tree->ScaleTo(initialScale, false);
        tree->SetMoverRestricted(false);
        tree->SetOrnamentsMovement(false);
        
        ornamentStore->Dismiss();
        bonusOrnamentStore->Dismiss();
        editorPanel->Dismiss();
        // EditorClosedEvent.invoke();

        API::RequestManager::RemoveTreeOrnamentsRequestListener(
            std::bind(&ChristmasTreeEditor::OnTreeRequestStateChanged, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void ChristmasTreeEditor::SetUploading(bool loading) {
        uploading = loading;
        editorPanel->SetLoading(loading);
    }

    void ChristmasTreeEditor::UploadSettings() {
        auto ornaments = tree->_ornaments;
        std::vector<ChristmasTreeOrnamentSettings> settings;

        for (int i = 0; i < ornaments->get_Count(); i++) {
            auto ornament = ornaments->get_Item(i);
            if (ornament != nullptr) {
                settings.push_back(ornament->GetSettings());
            }
        }

        API::RequestManager::SendUploadTreeOrnamentsRequest(settings);
    }

    void ChristmasTreeEditor::OnTreeRequestStateChanged(API::RequestState state, StringW result, StringW failReason) {
        switch (state) {
            case API::RequestState::Started:
                SetUploading(true);
                break;
            case API::RequestState::Finished:
                SetUploading(false);
                Dismiss();
                break;
            case API::RequestState::Failed:
                // getLogger().error("Tree request failed: %s", failReason.c_str());
                SetUploading(false);
                API::RequestManager::SendChristmasTreeRequest();
                Dismiss();
                break;
        }
    }

} // namespace BeatLeader