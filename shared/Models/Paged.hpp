#pragma once
#include <vector>
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

template<typename T>
class Paged {
public:
    class Metadata {
    public:
        int itemsPerPage;
        int page;
        int total;

        int GetPagesCount() const {
            return ceil(static_cast<float>(total) / itemsPerPage);
        }

        Metadata(rapidjson::Value const& document) {
            itemsPerPage = document["itemsPerPage"].GetInt();
            page = document["page"].GetInt();
            total = document["total"].GetInt();
        }

        Metadata() = default;
    };

    Metadata metadata;
    std::vector<T> data;
    std::optional<T> selection;

    Paged(rapidjson::Value const& document) : metadata(document["metadata"]) {
        auto dataArray = document["data"].GetArray();
        data.reserve(dataArray.Size());
        for (auto& item : dataArray) {
            data.emplace_back(T(item));
        }

        if (document.HasMember("selection") && !document["selection"].IsNull()) {
            selection = T(document["selection"]);
        }
    }

    Paged() = default;
}; 