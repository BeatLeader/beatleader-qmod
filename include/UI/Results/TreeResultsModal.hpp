// #pragma once

// #include "UI/Abstract/AbstractReeModal.hpp"
// #include "UnityEngine/GameObject.hpp"
// #include "UnityEngine/Transform.hpp"
// #include "UnityEngine/Vector3.hpp"
// #include "TMPro/TextMeshProUGUI.hpp"
// #include "Models/TreeStatus.hpp"
// #include "API/RequestManager.hpp"
// #include "custom-types/shared/coroutine.hpp"
// #include "System/Collections/Generic/List_1.hpp"
// #include <vector>

// DECLARE_CLASS_CODEGEN(BeatLeader, TreeResultsModalComponent, BeatLeader::ReeComponent,
//     DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, dateText);
//     DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, present);
//     DECLARE_INSTANCE_FIELD(bool, active);
//     DECLARE_INSTANCE_FIELD(bool, animating);
//     DECLARE_INSTANCE_FIELD(System::Collections::Generic::List_1<UnityEngine::GameObject*>*, spawned);

//     DECLARE_INSTANCE_METHOD(void, OnRootStateChange, bool active);
//     DECLARE_INSTANCE_METHOD(void, OnClosePressed);
//     DECLARE_INSTANCE_METHOD(void, OnDecoratePressed);

//     public:
//         std::vector<std::pair<int, std::string>> existingOrnamentIds;
//         std::vector<std::pair<int, std::string>> ornamentsToShow;
//         std::vector<UnityEngine::GameObject*> ornamentPrefabs;

//         void HandleTreeRequestState(API::RequestState state, TreeStatus* result, StringW failReason);
//         custom_types::Helpers::Coroutine AnimatePresent();
//         custom_types::Helpers::Coroutine AnimateOrnament(UnityEngine::GameObject* ornament, UnityEngine::Vector3 targetPos);
// )

// namespace BeatLeader {

// class TreeResultsModal : public AbstractReeModal<TreeResultsModalComponent*> {
// public:
//     void OnInitialize() override;
//     void OnDispose() override;
//     void OnResume() override;
//     StringW GetContent() override;
// };

// } // namespace BeatLeader 