#include "UI/MainMenu/DanceEvent/DanceEventModels.hpp"
#include "Assets/DanceEventAssets.hpp"
#include "main.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Rendering/IndexFormat.hpp"

#include "bsml/shared/Helpers/utilities.hpp"

#include <cstdlib>
#include <map>
#include <optional>
#include <unordered_map>

using namespace UnityEngine;

namespace DanceEventModels {
    #pragma region OBJ

    static void Split(std::string_view line, char separator, std::vector<std::string_view>& parts) {
        parts.clear();
        size_t start = 0;
        while (start <= line.size()) {
            size_t end = line.find(separator, start);
            if (end == std::string_view::npos) end = line.size();
            parts.push_back(line.substr(start, end - start));
            start = end + 1;
        }
    }

    static void Tokens(std::string_view line, std::vector<std::string_view>& tokens) {
        tokens.clear();
        size_t i = 0;
        while (i < line.size()) {
            while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) i++;
            size_t start = i;
            while (i < line.size() && line[i] != ' ' && line[i] != '\t') i++;
            if (i > start) tokens.push_back(line.substr(start, i - start));
        }
    }

    // The tokens are views into the null-terminated file text, strto* stops at their end
    static float ParseFloat(std::string_view token) {
        return std::strtof(token.data(), nullptr);
    }

    static int ParseIndex(std::string_view token, int count) {
        int index = (int)std::strtol(token.data(), nullptr, 10);
        return index > 0 ? index - 1 : count + index;
    }

    // OBJ is right-handed: x is mirrored and the winding flipped, like Unity's own OBJ import
    static Vector3 Mirrored(std::vector<std::string_view> const& tokens) {
        if (tokens.size() < 4) return Vector3(0, 0, 0);
        return Vector3(-ParseFloat(tokens[1]), ParseFloat(tokens[2]), ParseFloat(tokens[3]));
    }

    static std::optional<Model> Parse(std::string const& text, std::string const& name) {
        std::vector<Vector3> positions, normals;
        std::vector<Vector2> uvs;
        std::vector<Vector3> vertices, vertexNormals;
        std::vector<Vector2> vertexUvs;
        std::unordered_map<uint64_t, int> lookup;
        std::vector<std::pair<std::string, std::vector<int>>> groups;
        int current = -1;

        std::vector<std::string_view> tokens, ids;
        std::vector<int> corners;

        auto vertex = [&](std::string_view corner) -> int {
            Split(corner, '/', ids);
            int p = ParseIndex(ids[0], (int)positions.size());
            int t = ids.size() > 1 && !ids[1].empty() ? ParseIndex(ids[1], (int)uvs.size()) : -1;
            int n = ids.size() > 2 && !ids[2].empty() ? ParseIndex(ids[2], (int)normals.size()) : -1;
            if (p < 0 || p >= (int)positions.size() || t >= (int)uvs.size() || n >= (int)normals.size()) return -1;
            uint64_t key = (uint64_t)(p + 1) | ((uint64_t)(t + 1) << 21) | ((uint64_t)(n + 1) << 42);
            auto found = lookup.find(key);
            if (found != lookup.end()) return found->second;
            int index = (int)vertices.size();
            vertices.push_back(positions[p]);
            vertexUvs.push_back(t >= 0 ? uvs[t] : Vector2(0, 0));
            vertexNormals.push_back(n >= 0 ? normals[n] : Vector3(0, 1, 0));
            lookup.emplace(key, index);
            return index;
        };

        size_t position = 0;
        while (position < text.size()) {
            size_t end = text.find('\n', position);
            if (end == std::string::npos) end = text.size();
            std::string_view line(text.data() + position, end - position);
            position = end + 1;
            if (!line.empty() && line.back() == '\r') line.remove_suffix(1);

            Tokens(line, tokens);
            if (tokens.empty()) continue;
            auto const& kind = tokens[0];
            if (kind == "v") {
                positions.push_back(Mirrored(tokens));
            } else if (kind == "vn") {
                normals.push_back(Mirrored(tokens));
            } else if (kind == "vt") {
                if (tokens.size() >= 3) uvs.push_back(Vector2(ParseFloat(tokens[1]), ParseFloat(tokens[2])));
            } else if (kind == "usemtl") {
                std::string material = tokens.size() > 1 ? std::string(tokens[1]) : "";
                current = -1;
                for (size_t i = 0; i < groups.size(); i++) {
                    if (groups[i].first == material) current = (int)i;
                }
                if (current < 0) {
                    groups.emplace_back(material, std::vector<int>());
                    current = (int)groups.size() - 1;
                }
            } else if (kind == "f") {
                if (current < 0) {
                    groups.emplace_back("", std::vector<int>());
                    current = (int)groups.size() - 1;
                }
                corners.clear();
                bool valid = true;
                for (size_t i = 1; i < tokens.size(); i++) {
                    int index = vertex(tokens[i]);
                    if (index < 0) valid = false;
                    corners.push_back(index);
                }
                if (!valid) continue;
                auto& indices = groups[current].second;
                for (size_t i = 1; i + 1 < corners.size(); i++) {
                    indices.push_back(corners[0]);
                    indices.push_back(corners[i + 1]);
                    indices.push_back(corners[i]);
                }
            }
        }
        if (vertices.empty() || groups.empty()) return std::nullopt;

        auto mesh = Mesh::New_ctor();
        mesh->set_name(name);
        if (vertices.size() > 65000) mesh->set_indexFormat(Rendering::IndexFormat::UInt32);
        mesh->SetVertices(ArrayW<Vector3>(vertices));
        mesh->SetNormals(ArrayW<Vector3>(vertexNormals));
        mesh->SetUVs(0, ArrayW<Vector2>(vertexUvs));

        mesh->set_subMeshCount((int)groups.size());
        Model model;
        for (size_t i = 0; i < groups.size(); i++) {
            mesh->SetTriangles(ArrayW<int>(groups[i].second), (int)i);
            model.materials.push_back(groups[i].first);
        }
        mesh->RecalculateBounds();
        model.mesh = mesh;
        BeatLeaderLogger.info("Dance event model {}: {} vertices, {} parts", name, vertices.size(), groups.size());
        return model;
    }

    static std::map<std::string, std::optional<Model>> models;

    Model const* Get(std::string const& file) {
        auto found = models.find(file);
        if (found != models.end()) {
            if (!found->second) return nullptr;
            if (found->second->mesh) return &*found->second;
            models.erase(found);
        }
        auto text = DanceEventAssets::Gunzip(file);
        auto stored = models.emplace(file, text ? Parse(*text, file) : std::nullopt).first;
        if (!stored->second) BeatLeaderLogger.error("Couldn't load the model {}", file);
        return stored->second ? &*stored->second : nullptr;
    }

    #pragma endregion

    #pragma region Materials

    static Material* FindMaterial(SafePtrUnity<Material>& cache, std::string_view name) {
        if (cache) return cache.ptr();
        auto materials = Resources::FindObjectsOfTypeAll<Material*>();
        for (auto material : materials) {
            if (material && material->get_name() == name) {
                cache = material;
                return material;
            }
        }
        BeatLeaderLogger.warn("Dance event decor: no {} material", name);
        return nullptr;
    }

    static Material* Copy(Material* source, std::string_view name, Color color) {
        if (!source) return nullptr;
        auto material = Material::New_ctor(source);
        material->set_name(std::string("DanceEvent") + std::string(name));
        material->set_color(color);
        return material;
    }

    Material* Solid(Color color, float minimumLight) {
        static SafePtrUnity<Material> noteMenu;
        static int ambientMinimumId = Shader::PropertyToID("_AmbientMinimalValue");
        auto material = Copy(FindMaterial(noteMenu, "NoteMenu"), "NoteMenu", color);
        if (material) material->SetFloat(ambientMinimumId, minimumLight);
        return material;
    }

    Material* Glowing(Color color) {
        static SafePtrUnity<Material> cursor;
        return Copy(FindMaterial(cursor, "UICursor"), "UICursor", color);
    }

    Color Hex(std::string_view hex) {
        return BSML::Utilities::ParseHTMLColorOpt("#" + std::string(hex)).value_or(Color(1, 1, 1, 1));
    }

    #pragma endregion
}
