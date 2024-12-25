#pragma once

#include "Models/TreeStatus.hpp"
#include "Models/ChristmasTreeSettings.hpp"
#include "Models/SerializablePose.hpp"
#include "Models/ChristmasTreeOrnamentSettings.hpp"
#include "custom-types/shared/delegate.hpp"
#include <string>

namespace BeatLeader::API {

enum class RequestState {
    Started,
    Finished,
    Failed
};

using TreeStatusCallback = std::function<void(RequestState, TreeStatus*, StringW)>;
using TreeRequestCallback = std::function<void(RequestState, ChristmasTreeSettings*, StringW)>;
using TreeOrnamentsCallback = std::function<void(RequestState, StringW, StringW)>;
using TreeUploadCallback = std::function<void(RequestState, StringW, StringW)>;

class RequestManager {
public:
    static TreeStatus* treeStatus;
    static void SendTreeStatusRequest();
    static void SendOthersTreeRequest(std::string playerId);
    static void SendChristmasTreeRequest();
    static void SendUpdateTreeRequest(const FullSerializablePose& pose);
    static void SendUploadTreeOrnamentsRequest(const std::vector<ChristmasTreeOrnamentSettings>& ornaments);

    static void AddTreeStatusRequestListener(TreeStatusCallback callback);
    static void RemoveTreeStatusRequestListener(TreeStatusCallback callback);

    static void AddOthersTreeRequestListener(TreeRequestCallback callback);
    static void RemoveOthersTreeRequestListener(TreeRequestCallback callback);

    static void AddTreeRequestListener(TreeRequestCallback callback);
    static void RemoveTreeRequestListener(TreeRequestCallback callback);

    static void AddTreeUploadRequestListener(TreeUploadCallback callback);
    static void RemoveTreeUploadRequestListener(TreeUploadCallback callback);

    static void AddTreeOrnamentsRequestListener(TreeOrnamentsCallback callback);
    static void RemoveTreeOrnamentsRequestListener(TreeOrnamentsCallback callback);

private:
    static std::vector<TreeStatusCallback> treeStatusCallbacks;
    static std::vector<TreeRequestCallback> treeRequestCallbacks;
    static std::vector<TreeRequestCallback> othersTreeRequestCallbacks;
    static std::vector<TreeOrnamentsCallback> treeOrnamentsCallbacks;
    static std::vector<TreeUploadCallback> treeUploadCallbacks;

    static void NotifyTreeStatusListeners(RequestState state, TreeStatus* status, StringW failReason);
    static void NotifyTreeRequestListeners(RequestState state, ChristmasTreeSettings* settings, StringW failReason);
    static void NotifyTreeUploadListeners(RequestState state, StringW result, StringW failReason);
    static void NotifyOthersTreeRequestListeners(RequestState state, ChristmasTreeSettings* settings, StringW failReason);
    static void NotifyTreeOrnamentsListeners(RequestState state, StringW result, StringW failReason);
};

} // namespace BeatLeader::API 