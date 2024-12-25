// #include "UI/Results/TreeResultsModal.hpp"
// #include "UI/Christmas/ChristmasOrnamentLoader.hpp"
// #include "Assets/BundleLoader.hpp"
// #include "UnityEngine/Time.hpp"
// #include "UnityEngine/Mathf.hpp"
// #include "UnityEngine/Quaternion.hpp"
// #include "UnityEngine/Random.hpp"

// DEFINE_TYPE(BeatLeader, TreeResultsModalComponent);

// namespace BeatLeader {

//     void TreeResultsModal::OnInitialize() {
//         auto status = API::RequestManager::treeStatus;
//         if (status) {
//             LocalComponent()->existingOrnamentIds = status->GetOrnamentIds();
            
//             int ninetenthDay = 1734566400;
//             int day = 19 + (int)((ninetenthDay - status->today->startTime) / 86400);
//             std::string suffix = "th";
//             if (day % 10 == 1) suffix = "st";
//             else if (day % 10 == 2) suffix = "nd";
//             else if (day % 10 == 3) suffix = "rd";
            
//             LocalComponent()->dateText->set_text("December " + std::to_string(day) + suffix + " completed!");
//         }

//         API::RequestManager::AddTreeStatusRequestListener(
//             std::bind(&TreeResultsModalComponent::HandleTreeRequestState, LocalComponent(),
//                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//     }

//     void TreeResultsModal::OnDispose() {
//         API::RequestManager::RemoveTreeStatusRequestListener(
//             std::bind(&TreeResultsModalComponent::HandleTreeRequestState, LocalComponent(),
//                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//     }

//     void TreeResultsModal::OnResume() {
//         LocalComponent()->spawned = System::Collections::Generic::List_1<UnityEngine::GameObject*>::New_ctor();
//         if (LocalComponent()->present) {
//             UnityEngine::Object::Destroy(LocalComponent()->present);
//         }
//         LocalComponent()->active = false;
//         LocalComponent()->animating = false;
//         LocalComponent()->ornamentPrefabs.clear();
        
//         LocalComponent()->present = UnityEngine::Object::Instantiate(BundleLoader::bundle->present);
//         LocalComponent()->present->get_transform()->SetParent(LocalComponent()->get_transform(), false);
//         LocalComponent()->present->get_transform()->set_localPosition(
//             UnityEngine::Vector3::op_Addition(LocalComponent()->present->get_transform()->get_localPosition(), UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_up(), 20.0f)));
//     }

//     void TreeResultsModalComponent::HandleTreeRequestState(API::RequestState state, TreeStatus* result, StringW failReason) {
//         if (state != API::RequestState::Finished || !result) return;

//         ornamentsToShow.clear();
//         for (auto& ornament : result->GetOrnamentIds()) {
//             bool found = false;
//             for (auto& existing : existingOrnamentIds) {
//                 if (existing.first == ornament.first) {
//                     found = true;
//                     break;
//                 }
//             }
//             if (!found) {
//                 ornamentsToShow.push_back(ornament);
//             }
//         }

//         if (result->today->bundleId != 0) {
//             bool found = false;
//             for (auto& ornament : ornamentsToShow) {
//                 if (ornament.first == result->today->bundleId) {
//                     found = true;
//                     break;
//                 }
//             }
//             if (!found) {
//                 ornamentsToShow.push_back({result->today->bundleId, ""});
//             }
//         }

//         if (ornamentsToShow.empty()) {
//             // ResultsModal::AbstractReeModal::Close();
//         }
//     }

//     void TreeResultsModalComponent::OnRootStateChange(bool isActive) {
//         active = isActive;
//         if (active) {
//             StartCoroutine(custom_types::Helpers::CoroutineHelper::New(AnimatePresent()));
//         }
//     }

//     custom_types::Helpers::Coroutine TreeResultsModalComponent::AnimatePresent() {
//         if (animating) co_return;
//         animating = true;

//         // Clear previous spawned ornaments
//         for (int i = 0; i < spawned->get_Count(); i++) {
//             auto obj = spawned->get_Item(i);
//             UnityEngine::Object::Destroy(obj);
//         }
//         spawned->Clear();

//         float dropDuration = 0.8f;
//         float elapsedTime = 0;
//         auto startPos = present->get_transform()->get_localPosition();
//         auto targetPos = UnityEngine::Vector3::op_Subtraction(startPos, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_up(), 20.0f));
//         auto originalPos = targetPos;

//         while (elapsedTime < dropDuration) {
//             elapsedTime += UnityEngine::Time::get_deltaTime();
//             float progress = elapsedTime / dropDuration;
//             present->get_transform()->set_localPosition(
//                 UnityEngine::Vector3::Lerp(startPos, targetPos, progress));
//             co_yield nullptr;
//         }

//         API::RequestManager::SendTreeStatusRequest();

//         co_yield custom_types::MakeDelegate<System::Collections::IEnumerator*>(
//             (std::function<bool()>)[this]() {
//                 return !ornamentsToShow.empty();
//             });

//         // Load ornament prefabs
//         for (auto& ornament : ornamentsToShow) {
//             ChristmasOrnamentLoader::LoadOrnamentPrefabAsync(ornament.first, 
//                 [this](UnityEngine::GameObject* prefab) {
//                     ornamentPrefabs.push_back(prefab);
//                 });
//         }

//         co_yield custom_types::MakeDelegate<System::Collections::IEnumerator*>(
//             (std::function<bool()>)[this]() {
//                 return ornamentPrefabs.size() == ornamentsToShow.size();
//             });

//         present->get_transform()->set_localPosition(originalPos);

//         // Animate lid and blast
//         auto lid = present->get_transform()->Find("ChristmasPresentLid")->get_gameObject();
//         float openDuration = 0.3f;
//         elapsedTime = 0;
//         auto startRotation = lid->get_transform()->get_localRotation();
//         auto targetRotation = UnityEngine::Quaternion::op_Multiply(startRotation, UnityEngine::Quaternion::Euler(0, 0, 80));

//         while (elapsedTime < openDuration) {
//             elapsedTime += UnityEngine::Time::get_deltaTime();
//             float progress = elapsedTime / openDuration;
//             float angle = UnityEngine::Mathf::Lerp(0, 80, progress) + 
//                 UnityEngine::Mathf::Sin(progress * 3.1415f) * 20;
//             lid->get_transform()->set_localRotation(
//                 UnityEngine::Quaternion::op_Multiply(startRotation, UnityEngine::Quaternion::Euler(0, 0, angle)));
//             co_yield nullptr;
//         }

//         // Create blast effect
//         auto blast = UnityEngine::GameObject::New_ctor("Blast");
//         blast->get_transform()->SetParent(present->get_transform(), false);
//         blast->get_transform()->set_localPosition(UnityEngine::Vector3::get_zero());
//         blast->get_transform()->set_localScale(UnityEngine::Vector3::get_zero());

//         float blastDuration = 0.3f;
//         elapsedTime = 0;

//         while (elapsedTime < blastDuration) {
//             elapsedTime += UnityEngine::Time::get_deltaTime();
//             float progress = elapsedTime / blastDuration;
//             float scale = (1 - (progress * progress)) * 3.0f;
//             blast->get_transform()->set_localScale(
//                 UnityEngine::Vector3(scale, scale, scale));
//             co_yield nullptr;
//         }

//         UnityEngine::Object::Destroy(blast);

//         // Spawn and animate ornaments
//         for (auto prefab : ornamentPrefabs) {
//             auto ornament = UnityEngine::Object::Instantiate(prefab);
//             ornament->get_transform()->SetParent(get_transform());
//             ornament->get_transform()->set_position(present->get_transform()->get_position());
//             ornament->get_transform()->set_localScale(
//                 UnityEngine::Vector3::op_Multiply(ornament->get_transform()->get_localScale(), 4.0f));

//             float angle = UnityEngine::Random::Range(230.0f, 310.0f);
//             float radius = UnityEngine::Random::Range(0.3f, 0.8f);
//             UnityEngine::Vector3 targetPos = UnityEngine::Vector3::op_Addition(present->get_transform()->get_position(), 
//                 UnityEngine::Vector3(
//                     UnityEngine::Mathf::Cos(angle * (3.14159265358979f * 2.0f / 360.0f)) * radius,
//                     UnityEngine::Random::Range(0.3f, 0.8f),
//                     UnityEngine::Mathf::Sin(angle * (3.14159265358979f * 2.0f / 360.0f)) * radius
//                 ));

//             spawned->Add(ornament);
//             StartCoroutine(custom_types::Helpers::CoroutineHelper::New(AnimateOrnament(ornament, targetPos)));
//         }

//         co_return;
//     }

//     custom_types::Helpers::Coroutine TreeResultsModalComponent::AnimateOrnament(UnityEngine::GameObject* ornament, UnityEngine::Vector3 targetPos) {
//         float duration = 0.35f;
//         float elapsedTime = 0;
//         auto startPos = ornament->get_transform()->get_position();

//         while (elapsedTime < duration) {
//             elapsedTime += UnityEngine::Time::get_deltaTime();
//             float progress = elapsedTime / duration;
            
//             float height = UnityEngine::Mathf::Sin(progress * 3.1415f) * 0.8f;
//             float overshoot = UnityEngine::Mathf::Sin(progress * 3.1415f * 3) * 
//                 (1 - progress) * 0.3f;
//             auto currentPos = UnityEngine::Vector3::Lerp(startPos, targetPos, progress);
//             currentPos = UnityEngine::Vector3::op_Addition(currentPos, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::get_up(), height));
//             currentPos = UnityEngine::Vector3::op_Addition(currentPos, UnityEngine::Vector3::op_Multiply(UnityEngine::Vector3::op_Subtraction(targetPos, startPos).get_normalized(), overshoot));
            
//             ornament->get_transform()->set_position(currentPos);
//             co_yield nullptr;
//         }

//         ornament->get_transform()->set_position(targetPos);
//         co_return;
//     }

//     void TreeResultsModalComponent::OnClosePressed() {
//         // TreeResultsModal::AbstractReeModal::Close();
//     }

//     void TreeResultsModalComponent::OnDecoratePressed() {
//         // LeaderboardEvents::NotifyTreeEditorWasRequested();
//         // TreeResultsModal::AbstractReeModal::Close();
//     }

//     StringW TreeResultsModal::GetContent() {
//         return StringW(R"(
//             <vertical pad="2" spacing="3" vertical-fit="PreferredSize" bg="round-rect-panel">
//                 <text id="date-text" text="December 12th completed!" align="Center"/>
//                 <horizontal id="vertical-spacer" pref-height="20" />
//                 <horizontal id="button-container" pref-width="50" pad="2" spacing="0" horizontal-fit="PreferredSize" bg="round-rect-panel" bg-color="#ffffff88">
//                     <button id="button" pref-height="8" text=" Decorate the tree!" on-click="OnDecoratePressed" pad="0" />
//                     <vertical pref-width="2" vertical-fit="PreferredSize" />
//                     <button id="button" pref-height="8" text="Close" on-click="OnClosePressed" pad="0" />
//                 </horizontal>
//             </vertical>
//         )");
//     }

// } // namespace BeatLeader 