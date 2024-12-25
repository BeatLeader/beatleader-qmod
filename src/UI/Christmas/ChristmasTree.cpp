#include "UI/Christmas/ChristmasTree.hpp"
#include "UI/Christmas/ChristmasOrnamentLoader.hpp"
#include "UnityEngine/Debug.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Vector4.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "Utils/UnityExtensions.hpp"
#include "UI/Christmas/ChristmasTreeManager.hpp"

DEFINE_TYPE(BeatLeader, ChristmasTree);

namespace BeatLeader {

    void ChristmasTree::Present() {
        _animator->set_TargetScale(settings.gameTreePose.scale.x);
    }

    void ChristmasTree::Dismiss() {
        _animator->set_TargetScale(0.0f);
        StopSpinning();
    }

    void ChristmasTree::MoveTo(UnityEngine::Vector3 pos, bool immediate) {
        _animator->set_TargetPosition(pos);
        if (immediate) {
            _animator->EvaluatePosImmediate();
        }
    }

    void ChristmasTree::ScaleTo(float size, bool immediate) {
        _animator->set_TargetScale(size);
        if (immediate) {
            _animator->EvaluateScaleImmediate();
        }
    }

    void ChristmasTree::Awake() {
        _spinSpeed = 20.0f;
        _moverFull = false;
        _moverRestricted = false;
        _isSpinning = false;
        _ornaments = System::Collections::Generic::List_1<ChristmasTreeOrnament*>::New_ctor();
    }

    void ChristmasTree::SetOrnamentsMovement(bool value) {
        for (int i = 0; i < _ornaments->get_Count(); i++) {
            auto ornament = _ornaments->get_Item(i);
            if (ornament != nullptr) {
                ornament->canGrab = value;
            }
        }
    }

    void ChristmasTree::SetMoverFull(bool value) {
        _moverFull = value;
        _mover->SetEnabled(_moverFull, _moverRestricted);
    }

    void ChristmasTree::SetMoverRestricted(bool value) {
        _moverRestricted = value;
        _mover->SetEnabled(_moverFull, _moverRestricted);
    }

    void ChristmasTree::StartSpinning() {
        _isSpinning = true;
    }

    void ChristmasTree::StopSpinning() {
        _isSpinning = false;
    }

    void ChristmasTree::Update() {
        auto shaderPos = get_transform()->get_position();
        shaderPos.y += 1.5f;
        UnityEngine::Shader::SetGlobalVector(UnityEngine::Shader::PropertyToID("_TreePosition"), 
            UnityEngine::Vector4(shaderPos.x, shaderPos.y, shaderPos.z, 1.0f));
        
        if (_isSpinning) {
            get_transform()->Rotate(UnityEngine::Vector3::get_up(), _spinSpeed * UnityEngine::Time::get_deltaTime());
        }
    }

    void ChristmasTree::LoadSettings(ChristmasTreeSettings newSettings, bool move) {
        settings = newSettings;
        LoadOrnaments(settings, 0, [this, move]() {
            
        });
        if (move) {
            MoveTo(settings.gameTreePose.position, true);
            ScaleTo(settings.gameTreePose.scale.x, true);
        }
    }

    void ChristmasTree::AddOrnament(ChristmasTreeOrnament* ornament) {
        _ornaments->Add(ornament);
    }

    void ChristmasTree::RemoveOrnament(ChristmasTreeOrnament* ornament) {
        _ornaments->Remove(ornament);
    }

    void ChristmasTree::ClearOrnaments() {
        for (int i = 0; i < _ornaments->get_Count(); i++) {
            auto l = _ornaments->get_Item(i);
            UnityEngine::Object::Destroy(l->get_gameObject());
        }
        _ornaments->Clear();
    }

    void ChristmasTree::LoadOrnaments(ChristmasTreeSettings settings, int index, std::function<void()> callback) {
        if (index == 0) {
            ClearOrnaments();
        }

        // for (auto& ornament : settings.ornaments) {
        //     co_yield custom_types::Helpers::CoroutineHelper::New(
        //         ornamentsPool->PreloadAsync(ornament.bundleId));
        // }

        if (settings.ornaments.size() == 0) return;

        ChristmasOrnamentLoader::Spawn(settings.ornaments[index].bundleId, this, nullptr, [this, index, callback, settings](ChristmasTreeOrnament* ornament) {
            ornament->get_transform()->SetParent(get_transform(), false);
            UnityExtensions::SetLocalPose(ornament->get_transform(), settings.ornaments[index].pose);
            _ornaments->Add(ornament);
            if (index == settings.ornaments.size() - 1) {
                callback();
            } else {
                LoadOrnaments(settings, index + 1, callback);
            }
        });
    }

    UnityEngine::Vector3 ChristmasTree::Align(UnityEngine::Vector3 pos) {
        float y = pos.y;
        
        ChristmasTreeLevel* level = nullptr;
        for (int i = 0; i < _levels->get_Count(); i++) {
            auto l = _levels->get_Item(i);
            if (y >= l->bottomHeight && y <= l->topHeight) {
                level = l;
                break;
            }
        }

        if (!level) {
            return pos;
        }

        float t = (y - level->bottomHeight) / (level->topHeight - level->bottomHeight);
        float radiusAtHeight = UnityEngine::Mathf::Lerp(level->bottomRadius, level->topRadius, t);

        UnityEngine::Vector2 xz(pos.x, pos.z);
        float magnitude = xz.magnitude;
        if (magnitude == 0) {
            xz = UnityEngine::Vector2(1, 0);
        } else {
            xz = UnityEngine::Vector2::op_Division(xz, magnitude);
        }
        xz = UnityEngine::Vector2::op_Multiply(xz, radiusAtHeight);

        return UnityEngine::Vector3(xz.x, pos.y, xz.y);
    }

    bool ChristmasTree::HasAreaContact(UnityEngine::Vector3 pos) {
        auto mul = _mesh->get_localScale();
        pos = _mesh->InverseTransformPoint(pos);
        return std::abs(pos.x) <= _radius * mul.x && std::abs(pos.z) <= _radius * mul.z;
    }

    void ChristmasTree::OnPointerDown(UnityEngine::EventSystems::PointerEventData* eventData) {
        if (!_hovered) return;

        UnityEngine::EventSystems::BaseInputModule* baseModule = eventData->get_currentInputModule();
        auto module = reinterpret_cast<VRUIControls::VRInputModule*>(baseModule);
        if (!module) return;

        ChristmasTreeManager::HandleTreeButtonClicked();
    }

    void ChristmasTree::OnPointerExit(UnityEngine::EventSystems::PointerEventData* eventData) {
        _hovered = false;
    }

    void ChristmasTree::OnPointerEnter(UnityEngine::EventSystems::PointerEventData* eventData) {
        _hovered = true;
    }

} // namespace BeatLeader
  