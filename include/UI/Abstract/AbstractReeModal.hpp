#pragma once

#include "HMUI/ImageView.hpp"
#include "UI/ReeUIComponentV2.hpp"
#include "UI/Abstract/IReeModal.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Action.hpp"
#include "main.hpp"

namespace BeatLeader {

template<typename C = BeatLeader::ReeComponent*>
requires(std::is_convertible_v<C, BeatLeader::ReeComponent*>) 
class AbstractReeModal : public ReeUIComponentV2<C>, public IReeModal {
protected:
    System::Action* closeAction;
    bool offClickCloses;

public:
    virtual void OnInitialize() override {
        auto* bg = this->LocalComponent()->_content->template GetComponent<HMUI::ImageView*>();
        if (bg != nullptr) {
            bg->set_raycastTarget(true);
        }
        offClickCloses = true;
    }

    virtual void OnContextChanged() {
        // Virtual - implemented by derived classes
    }

    virtual void OnResume() {
        // Virtual - implemented by derived classes
    }

    virtual void OnPause() {
        // Virtual - implemented by derived classes
    }

    virtual void OnInterrupt() {
        // Virtual - implemented by derived classes
    }

    virtual void OnClose() {
        // Virtual - implemented by derived classes
    }

    virtual void OnOffClick() {
        if (offClickCloses) {
            Close();
        }
    }

    void ClearContext() {
        // Context handling will be implemented by derived classes
    }

    virtual void SetContext(Il2CppObject* context) {
        // Context handling will be implemented by derived classes
        OnContextChanged();
    }
    
    // IReeModal implementation
    void Resume(void* state, System::Action* closeAction) override {
        this->closeAction = closeAction;
        SetContext(reinterpret_cast<Il2CppObject*>(state));
        this->LocalComponent()->get_gameObject()->SetActive(true);
        this->LocalComponent()->_content->get_gameObject()->SetActive(true);
        OnResume();
    }

    void Pause() override {
        this->LocalComponent()->get_gameObject()->SetActive(false);
        this->LocalComponent()->_content->get_gameObject()->SetActive(false);
        OnPause();
    }

    void Interrupt() override {
        this->LocalComponent()->get_gameObject()->SetActive(false);
        this->LocalComponent()->_content->get_gameObject()->SetActive(false);
        OnInterrupt();
    }

    void Close() override {
        if (closeAction != nullptr) {
            closeAction->Invoke();
        }
        OnClose();
    }

    void HandleOffClick() override {
        OnOffClick();
    }
};

} // namespace BeatLeader