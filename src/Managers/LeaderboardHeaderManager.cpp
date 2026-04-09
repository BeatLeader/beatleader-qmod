#include "include/Managers/LeaderboardHeaderManager.hpp"

#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

#include "include/Enhancers/MapEnhancer.hpp"
#include "include/Utils/WebUtils.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include <regex>

namespace {
    UnityEngine::Color FunnyColor = UnityEngine::Color(0.64f, 0.64f, 0.64f, 1.0f);
    UnityEngine::Color FunnyColor0 = UnityEngine::Color(0.93f, 0.0f, 0.55f, 1.0f);
    UnityEngine::Color FunnyColor1 = UnityEngine::Color(0.25f, 0.52f, 0.9f, 1.0f);

    UnityEngine::Color BoringColor = UnityEngine::Color(0.5f, 0.5f, 0.5f, 1.0f);
}

namespace BeatLeader {
    namespace LeaderboardHeaderManagerNS {
        LeaderboardHeaderManager Instance {};
    }

    void LeaderboardHeaderManager::EnsureInitialized(GlobalNamespace::PlatformLeaderboardViewController* controller,
                                                     std::function<void()> websiteCallback,
                                                     std::function<void()> settingsCallback) {
        if (_infoPanel) {
            _infoPanel->SetCallbacks(std::move(websiteCallback), std::move(settingsCallback));
            return;
        }

        UnityEngine::GameObject* header = nullptr;
        if (!TryFindHeader(controller, header)) {
            return;
        }

        _header = header;
        _headerText = header->GetComponentInChildren<TMPro::TextMeshProUGUI*>();
        _headerImage = header->GetComponentInChildren<HMUI::ImageView*>();

        if (!_headerImage) {
            return;
        }

        auto* infoPanel = BeatLeader::LeaderboardInfoPanel::Instantiate<BeatLeader::LeaderboardInfoPanel>(_headerImage->get_transform());
        infoPanel->LocalComponent()->ManualInit(_headerImage->get_transform());
        infoPanel->SetCallbacks(std::move(websiteCallback), std::move(settingsCallback));
        infoPanel->SetActive(false);

        ::UnityW<::UnityEngine::Transform> leaderboardTransform = controller->get_transform();
        auto* mapDifficultyPanel = BeatLeader::MapDifficultyPanel::Instantiate<BeatLeader::MapDifficultyPanel>(leaderboardTransform);
        mapDifficultyPanel->LocalComponent()->ManualInit(leaderboardTransform);
        mapDifficultyPanel->Clear();

        infoPanel->SetMapStatusHoverCallback([this](UnityEngine::Vector3 const& worldPos, bool isHovered, float progress) {
            if (_mapDifficultyPanel) {
                _mapDifficultyPanel->SetHoverState(worldPos, isHovered, progress);
            }
        });

        _mapDifficultyPanel = mapDifficultyPanel;
        _infoPanel = infoPanel;
        ApplyCurrentBeatmapData();
    }

    void LeaderboardHeaderManager::SetBeatmap(GlobalNamespace::BeatmapKey beatmapKey) {
        std::pair<std::string, std::string> lookupKey;
        if (!TryGetBeatmapLookupKey(beatmapKey, lookupKey)) {
            _currentLookupKey.reset();
            ++_requestId;
            ClearCurrentBeatmapData();
            return;
        }

        _currentLookupKey = lookupKey;

        if (_mapInfos.contains(lookupKey.first)) {
            ApplyCurrentBeatmapData();
            return;
        }

        ++_requestId;
        auto requestId = _requestId;
        ClearCurrentBeatmapData();

        auto url = WebUtils::API_URL + "map/modinterface/" + lookupKey.first;
        WebUtils::GetAsync(url, [this, lookupKey, requestId](long status, std::string stringResult) {
            BSML::MainThreadScheduler::Schedule([this, lookupKey, requestId, status, stringResult = std::move(stringResult)]() {
                if (requestId != _requestId) {
                    return;
                }

                if (!_currentLookupKey.has_value() || _currentLookupKey.value() != lookupKey) {
                    return;
                }

                if (status != 200) {
                    ClearCurrentBeatmapData();
                    return;
                }

                rapidjson::Document result;
                result.Parse(stringResult.c_str());
                if (result.HasParseError()) {
                    ClearCurrentBeatmapData();
                    return;
                }

                _mapInfos.insert_or_assign(lookupKey.first, Song(result));
                ApplyCurrentBeatmapData();
            });
        });
    }

    void LeaderboardHeaderManager::SetVisible(bool visible) {
        if (_infoPanel) {
            _infoPanel->SetActive(visible);
            if (visible) {
                _infoPanel->RefreshMapStatus();
            }
        }
        if (!visible && _mapDifficultyPanel) {
            _mapDifficultyPanel->Clear();
        }

        if (_headerText) {
            _headerText->get_gameObject()->SetActive(!visible);
        }

        ApplyColors(visible);
    }

    void LeaderboardHeaderManager::Reset() {
        _header = nullptr;
        _headerImage = nullptr;
        _headerText = nullptr;
        _mapDifficultyPanel = nullptr;
        _infoPanel = nullptr;
        _currentLookupKey.reset();
        ++_requestId;
    }

    void LeaderboardHeaderManager::RefreshMapStatus() {
        if (_infoPanel) {
            _infoPanel->RefreshMapStatus();
        }
        if (_mapDifficultyPanel) {
            _mapDifficultyPanel->RefreshVisuals();
        }
    }

    void LeaderboardHeaderManager::ApplyVoteToCurrentBeatmap(bool rankable, int type) {
        if (!_currentLookupKey.has_value()) {
            return;
        }

        auto songIt = _mapInfos.find(_currentLookupKey->first);
        if (songIt == _mapInfos.end()) {
            return;
        }

        auto diffIt = songIt->second.difficulties.find(_currentLookupKey->second);
        if (diffIt == songIt->second.difficulties.end()) {
            return;
        }

        if (type > 0) {
            diffIt->second.type |= type;
        }

        diffIt->second.votes.push_back(rankable ? 1.0f : 0.0f);
        ApplyCurrentBeatmapData();
    }

    bool LeaderboardHeaderManager::TryFindHeader(GlobalNamespace::PlatformLeaderboardViewController* controller, UnityEngine::GameObject*& header) {
        if (!controller) {
            header = nullptr;
            return false;
        }

        UnityW< ::UnityEngine::Transform> headerTransform = controller->get_transform()->Find("HeaderPanel");
        if (!headerTransform) {
            header = nullptr;
            return false;
        }

        header = headerTransform->get_gameObject();
        return header != nullptr;
    }

    bool LeaderboardHeaderManager::TryGetBeatmapLookupKey(GlobalNamespace::BeatmapKey beatmapKey, std::pair<std::string, std::string>& key) const {
        if (!beatmapKey.beatmapCharacteristic) {
            return false;
        }

        auto levelId = static_cast<std::string>(beatmapKey.levelId);
        auto hash = std::regex_replace(levelId, std::basic_regex("custom_level_"), "");
        auto difficulty = MapEnhancer::DiffName(beatmapKey.difficulty.value__);
        auto mode = static_cast<std::string>(beatmapKey.beatmapCharacteristic->serializedName);

        if (levelId.empty() || hash.empty() || difficulty.empty() || mode.empty()) {
            return false;
        }

        key = {hash, difficulty + mode};
        return true;
    }

    void LeaderboardHeaderManager::ApplyColors(bool funny) {
        if (!_headerImage) {
            return;
        }

        if (funny) {
            _headerImage->set_color(FunnyColor);
            _headerImage->set_color0(FunnyColor0);
            _headerImage->set_color1(FunnyColor1);
            return;
        }

        _headerImage->set_color(BoringColor);
        _headerImage->set_color0(BoringColor);
        _headerImage->set_color1(BoringColor);
    }

    void LeaderboardHeaderManager::ApplyCurrentBeatmapData() {
        if (!_infoPanel) {
            return;
        }

        if (!_currentLookupKey.has_value()) {
            ClearCurrentBeatmapData();
            return;
        }

        auto songIt = _mapInfos.find(_currentLookupKey->first);
        if (songIt == _mapInfos.end()) {
            ClearCurrentBeatmapData();
            return;
        }

        auto diffIt = songIt->second.difficulties.find(_currentLookupKey->second);
        if (diffIt == songIt->second.difficulties.end()) {
            ClearCurrentBeatmapData();
            return;
        }

        _infoPanel->SetMapDifficulty(diffIt->second);
        if (_mapDifficultyPanel) {
            _mapDifficultyPanel->SetDifficulty(diffIt->second);
        }
    }

    void LeaderboardHeaderManager::ClearCurrentBeatmapData() {
        if (_infoPanel) {
            _infoPanel->ClearBeatmapData();
        }
        if (_mapDifficultyPanel) {
            _mapDifficultyPanel->Clear();
        }
    }
}
