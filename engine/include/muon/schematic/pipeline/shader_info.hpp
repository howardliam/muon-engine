#pragma once

#include <cstdint>
#include <filesystem>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <string>

namespace muon::schematic {

    struct ShaderInfo {
        std::optional<std::filesystem::path> path{std::nullopt};
        std::optional<uint64_t> byteOffset{std::nullopt};
        std::optional<uint64_t> byteLength{std::nullopt};

        std::string entryPoint{};

        std::optional<glm::uvec3> workGroupSize{std::nullopt};

        [[nodiscard]] auto IsValid() const -> bool;
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ShaderInfo> {
        static auto to_json(json &j, const auto &info) {
            if (info.path) {
                j["path"] = info.path->string();
            } else if (info.byteOffset && info.byteLength) {
                j["byteOffset"] = *info.byteOffset;
                j["byteLength"] = *info.byteLength;
            }

            j["entryPoint"] = info.entryPoint;

            if (info.workGroupSize) {
                j["workGroupSize"][0] = info.workGroupSize->x;
                j["workGroupSize"][1] = info.workGroupSize->y;
                j["workGroupSize"][2] = info.workGroupSize->z;
            }
        }

        static auto from_json(const json &j, auto &info) {
            if (j.contains("path")) {
                info.path = { j["path"].get<std::filesystem::path>() };
            } else if (j.contains("byteOffset") && j.contains("byteLength")) {
                info.byteOffset = { j["byteOffset"].get<uint64_t>() };
                info.byteLength = { j["byteLength"].get<uint64_t>() };
            }

            info.entryPoint = j["entryPoint"];

            if (j.contains("workGroupSize") && j["workGroupSize"].is_array() && j["workGroupSize"].size() == 3) {
                info.workGroupSize = {
                    j["workGroupSize"][0].get<uint32_t>(),
                    j["workGroupSize"][1].get<uint32_t>(),
                    j["workGroupSize"][2].get<uint32_t>(),
                };
            }
        }
    };

}
