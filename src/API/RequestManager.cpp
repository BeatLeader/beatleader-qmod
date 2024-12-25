#include "API/RequestManager.hpp"
#include "Utils/WebUtils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

namespace BeatLeader::API {

    std::vector<TreeStatusCallback> RequestManager::treeStatusCallbacks;
    std::vector<TreeRequestCallback> RequestManager::treeRequestCallbacks;
    std::vector<TreeUploadCallback> RequestManager::treeUploadCallbacks;
    std::vector<TreeRequestCallback> RequestManager::othersTreeRequestCallbacks;
    std::vector<TreeOrnamentsCallback> RequestManager::treeOrnamentsCallbacks;

    TreeStatus* RequestManager::treeStatus;

    void RequestManager::SendTreeStatusRequest() {
        NotifyTreeStatusListeners(RequestState::Started, nullptr, "");

        WebUtils::GetJSONAsync(WebUtils::API_URL + "projecttree/status", 
            [](long status, bool error, rapidjson::Document const& response) {
                if (error || status != 200) {
                    NotifyTreeStatusListeners(RequestState::Failed, nullptr, 
                        "Failed to get tree status: " + std::to_string(status));
                    return;
                }

                try {
                    treeStatus = new TreeStatus(response);
                    NotifyTreeStatusListeners(RequestState::Finished, treeStatus, "");
                } catch (const std::exception& e) {
                    NotifyTreeStatusListeners(RequestState::Failed, nullptr, 
                        "Failed to parse tree status: " + std::string(e.what()));
                }
            });
    }

    void RequestManager::SendChristmasTreeRequest() {
        NotifyTreeRequestListeners(RequestState::Started, nullptr, "");

        WebUtils::GetJSONAsync(WebUtils::API_URL + "projecttree", 
            [](long status, bool error, rapidjson::Document const& response) {
                if (error || status != 200) {
                    NotifyTreeRequestListeners(RequestState::Failed, nullptr, 
                        "Failed to get tree: " + std::to_string(status));
                    return;
                }

                try {
                    auto treeSettings = new ChristmasTreeSettings(response);
                    NotifyTreeRequestListeners(RequestState::Finished, treeSettings, "");
                } catch (const std::exception& e) {
                    NotifyTreeRequestListeners(RequestState::Failed, nullptr, 
                        "Failed to parse tree: " + std::string(e.what()));
                }
            });
    }

    void RequestManager::SendOthersTreeRequest(std::string playerId) {
        NotifyOthersTreeRequestListeners(RequestState::Started, nullptr, "");

        WebUtils::GetJSONAsync(WebUtils::API_URL + "projecttree/" + playerId, 
            [](long status, bool error, rapidjson::Document const& response) {
                if (error || status != 200) {
                    NotifyOthersTreeRequestListeners(RequestState::Failed, nullptr, 
                        "Failed to get tree: " + std::to_string(status));
                    return;
                }

                try {
                    auto treeSettings = new ChristmasTreeSettings(response);
                    NotifyOthersTreeRequestListeners(RequestState::Finished, treeSettings, "");
                } catch (const std::exception& e) {
                    NotifyOthersTreeRequestListeners(RequestState::Failed, nullptr, 
                        "Failed to parse tree: " + std::string(e.what()));
                }
            });
    }

    void RequestManager::SendUpdateTreeRequest(const FullSerializablePose& pose) {
        NotifyTreeUploadListeners(RequestState::Started, "", "");

        WebUtils::PostJSONInBodyAsync(WebUtils::API_URL + "projecttree/game", pose.ToJSON(), 200,
            [](long status, string response) {
                if (status != 200) {
                    NotifyTreeUploadListeners(RequestState::Failed, "", "Failed to update tree: " + std::to_string(status));
                    return;
                }

                NotifyTreeUploadListeners(RequestState::Finished, "", "");
            });
    }

    void writeDocumentToString(const rapidjson::Document& document,
                           std::string& output)
    {
        class StringHolder
        {
        public:
            typedef char Ch;
            StringHolder(std::string& s) : s_(s) { s_.reserve(4096); }
            void Put(char c) { s_.push_back(c); }
            void Flush() { return; }

        private:
            std::string& s_;
        };

        StringHolder os(output);
        rapidjson::Writer<StringHolder> writer(os);
        document.Accept(writer);
    }

    void RequestManager::SendUploadTreeOrnamentsRequest(const std::vector<ChristmasTreeOrnamentSettings>& ornaments) {
        
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        doc.SetArray();

        for (const auto& ornament : ornaments) {
            rapidjson::Value ornamentObj(rapidjson::kObjectType);

            ornamentObj.AddMember("bundleId", ornament.bundleId, allocator);
            ornamentObj.AddMember("state", ornament.state, allocator);

            rapidjson::Value poseObj(rapidjson::kObjectType);
            
            rapidjson::Value positionObj(rapidjson::kObjectType);
            positionObj.AddMember("x", ornament.pose.position.x, allocator);
            positionObj.AddMember("y", ornament.pose.position.y, allocator); 
            positionObj.AddMember("z", ornament.pose.position.z, allocator);
            poseObj.AddMember("position", positionObj, allocator);

            rapidjson::Value rotationObj(rapidjson::kObjectType);
            rotationObj.AddMember("x", ornament.pose.rotation.x, allocator);
            rotationObj.AddMember("y", ornament.pose.rotation.y, allocator);
            rotationObj.AddMember("z", ornament.pose.rotation.z, allocator);
            rotationObj.AddMember("w", ornament.pose.rotation.w, allocator);
            poseObj.AddMember("rotation", rotationObj, allocator);

            ornamentObj.AddMember("pose", poseObj, allocator);
            doc.PushBack(ornamentObj, allocator);
        }

        std::string output;
        writeDocumentToString(doc, output);

        WebUtils::PostJSONInBodyAsync(WebUtils::API_URL + "projecttree/ornaments", output, 200,
            [](long status, string response) {
                if (status != 200) {
                    NotifyTreeOrnamentsListeners(RequestState::Failed, "", "Failed to upload tree ornaments: " + std::to_string(status));
                    return;
                }

                NotifyTreeOrnamentsListeners(RequestState::Finished, "", "");
            });
    }

    void RequestManager::AddTreeStatusRequestListener(TreeStatusCallback callback) {
        treeStatusCallbacks.push_back(callback);
    }

    void RequestManager::RemoveTreeStatusRequestListener(TreeStatusCallback callback) {
        // auto it = std::find(treeStatusCallbacks.begin(), treeStatusCallbacks.end(), callback);
        // if (it != treeStatusCallbacks.end()) {
        //     treeStatusCallbacks.erase(it);
        // }
    }

    void RequestManager::AddOthersTreeRequestListener(TreeRequestCallback callback) {
        othersTreeRequestCallbacks.push_back(callback);
    }

    void RequestManager::RemoveOthersTreeRequestListener(TreeRequestCallback callback) {
        // auto it = std::find(othersTreeRequestCallbacks.begin(), othersTreeRequestCallbacks.end(), callback);
        // if (it != othersTreeRequestCallbacks.end()) {
        //     othersTreeRequestCallbacks.erase(it);
        // }
    }

    void RequestManager::AddTreeRequestListener(TreeRequestCallback callback) {
        treeRequestCallbacks.push_back(callback);
    }

    void RequestManager::RemoveTreeRequestListener(TreeRequestCallback callback) {
        // auto it = std::find(treeRequestCallbacks.begin(), treeRequestCallbacks.end(), callback);
        // if (it != treeRequestCallbacks.end()) {
        //     treeRequestCallbacks.erase(it);
        // }
    }

    void RequestManager::AddTreeUploadRequestListener(TreeUploadCallback callback) {
        treeUploadCallbacks.push_back(callback);
    }

    void RequestManager::RemoveTreeUploadRequestListener(TreeUploadCallback callback) {
        // auto it = std::find(treeUploadCallbacks.begin(), treeUploadCallbacks.end(), callback);
        // if (it != treeUploadCallbacks.end()) {
        //     treeUploadCallbacks.erase(it);
        // }
    }
    
    void RequestManager::AddTreeOrnamentsRequestListener(TreeOrnamentsCallback callback) {
        treeOrnamentsCallbacks.push_back(callback);
    }

    void RequestManager::RemoveTreeOrnamentsRequestListener(TreeOrnamentsCallback callback) {
        // auto it = std::find(treeOrnamentsCallbacks.begin(), treeOrnamentsCallbacks.end(), callback);
        // if (it != treeOrnamentsCallbacks.end()) {
        //     treeOrnamentsCallbacks.erase(it);
        // }
    }

    void RequestManager::NotifyTreeStatusListeners(RequestState state, TreeStatus* status, StringW failReason) {
        BSML::MainThreadScheduler::Schedule([state, status, failReason]() {
            for (const auto& callback : treeStatusCallbacks) {
                callback(state, status, failReason);
            }
        });
    }

    void RequestManager::NotifyOthersTreeRequestListeners(RequestState state, ChristmasTreeSettings* settings, StringW failReason) {
        BSML::MainThreadScheduler::Schedule([state, settings, failReason]() {
            for (const auto& callback : othersTreeRequestCallbacks) {
                callback(state, settings, failReason);
            }
        });
    }

    void RequestManager::NotifyTreeUploadListeners(RequestState state, StringW result, StringW failReason) {
        BSML::MainThreadScheduler::Schedule([state, result, failReason]() {
            for (const auto& callback : treeUploadCallbacks) {
                callback(state, result, failReason);
            }
        });
    }

    void RequestManager::NotifyTreeRequestListeners(RequestState state, ChristmasTreeSettings* settings, StringW failReason) {
        BSML::MainThreadScheduler::Schedule([state, settings, failReason]() {
            for (const auto& callback : treeRequestCallbacks) {
                callback(state, settings, failReason);
            }
        });
    }

    void RequestManager::NotifyTreeOrnamentsListeners(RequestState state, StringW result, StringW failReason) {
        BSML::MainThreadScheduler::Schedule([state, result, failReason]() {
            for (const auto& callback : treeOrnamentsCallbacks) {
                callback(state, result, failReason);
            }
        });
    }

} // namespace BeatLeader::API 