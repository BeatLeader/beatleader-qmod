#include "UI/Christmas/ChristmasTreeOrnament.hpp"
#include "UI/Christmas/ChristmasTree.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/RigidbodyInterpolation.hpp"
#include "logging.hpp"
#include "Utils/UnityExtensions.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTreeOrnament);

namespace BeatLeader {

    void ChristmasTreeOrnament::SetupTree(UnityEngine::MonoBehaviour* treeInstance, int ornamentBundleId) {
        tree = treeInstance;
        bundleId = ornamentBundleId;
        initialized = true;
    }

    ChristmasTreeOrnamentSettings ChristmasTreeOrnament::GetSettings() {
        return ChristmasTreeOrnamentSettings(bundleId, UnityExtensions::GetLocalPose(get_transform()));
    }

    void ChristmasTreeOrnament::Init(UnityEngine::Transform* parent) {
        get_transform()->SetParent(parent, false);
        get_gameObject()->SetActive(true);
    }

    void ChristmasTreeOrnament::Awake() {
        rigidbody = get_gameObject()->AddComponent<UnityEngine::Rigidbody*>();
        if (rigidbody) {
            rigidbody->set_useGravity(false);
            rigidbody->set_interpolation(UnityEngine::RigidbodyInterpolation::None);
        }
    }

    void ChristmasTreeOrnament::Update() {
        if (!initialized) return;

        if (!grabbed) {
            if (hadContact) {
                float t = UnityEngine::Time::get_deltaTime() * 7.0f;
                auto pos = UnityEngine::Vector3::Lerp(get_transform()->get_localPosition(), alignedOrnamentPos, t);
                if (pos.x - alignedOrnamentPos.x < 0.001f) {
                    pos = alignedOrnamentPos;
                    hadContact = false;
                }
                get_transform()->set_localPosition(pos);
            } else if (get_transform()->get_position().y < 0) {
                UnityEngine::Object::Destroy(this->get_gameObject());
            }
        } else {
            get_transform()->set_position(grabbingController->TransformPoint(grabPos));
            get_transform()->set_rotation(UnityEngine::Quaternion::op_Multiply(grabbingController->get_rotation(), grabRot));
            controllerVelocity = UnityEngine::Vector3::op_Subtraction(grabbingController->get_position(), controllerLastPos);
            controllerLastPos = grabbingController->get_position();
        }
    }

    void ChristmasTreeOrnament::OnPointerDown(UnityEngine::EventSystems::PointerEventData* eventData) {
        if (!hovered) return;

        UnityEngine::EventSystems::BaseInputModule* baseModule = eventData->get_currentInputModule();
        auto module = reinterpret_cast<VRUIControls::VRInputModule*>(baseModule);
        if (!module) return;

        grabbingController = module->_vrPointer->lastSelectedVrController->get_transform();
        grabPos = grabbingController->InverseTransformPoint(get_transform()->get_position());
        
        if (grabPos.magnitude > MAX_DISTANCE) {
            grabPos = UnityEngine::Vector3::op_Multiply(grabPos.get_normalized(), MAX_DISTANCE);
        }

        grabRot = UnityEngine::Quaternion::op_Multiply(UnityEngine::Quaternion::Inverse(grabbingController->get_rotation()), get_transform()->get_rotation());
        if (rigidbody) {
            rigidbody->set_useGravity(false);
        }
        grabbed = true;

        if (_grabbedAction) {
            _grabbedAction->Invoke();
        }
    }

    void ChristmasTreeOrnament::OnPointerUp(UnityEngine::EventSystems::PointerEventData* eventData) {
        if (!grabbed) return;

        ChristmasTree* christmasTree = reinterpret_cast<ChristmasTree*>(tree);

        grabbingController = nullptr;
        hadContact = christmasTree->HasAreaContact(get_transform()->get_position());
        grabbed = false;

        if (hadContact) {
            get_transform()->SetParent(christmasTree->get_Origin(), true);
            alignedOrnamentPos = get_transform()->get_localPosition();
            christmasTree->AddOrnament(this);
        } else {
            christmasTree->RemoveOrnament(this);
            if (rigidbody) {
                rigidbody->set_interpolation(UnityEngine::RigidbodyInterpolation::Interpolate);
                rigidbody->set_useGravity(true);
                rigidbody->set_velocity(UnityEngine::Vector3::op_Multiply(controllerVelocity, 100.0f));
            } else {
                UnityEngine::Object::Destroy(this->get_gameObject());
            }
            // getLogger().error("VELOCITY: %f, %f, %f", controllerVelocity.x, controllerVelocity.y, controllerVelocity.z);
            
        }
    }

    void ChristmasTreeOrnament::OnPointerEnter(UnityEngine::EventSystems::PointerEventData* eventData) {
        hovered = true;
    }

    void ChristmasTreeOrnament::OnPointerExit(UnityEngine::EventSystems::PointerEventData* eventData) {
        hovered = false;
    }

} // namespace BeatLeader 