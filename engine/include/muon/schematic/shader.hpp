#pragma once

#include "muon/core/assert.hpp"
#include <cstdint>
#include <filesystem>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <string>

namespace muon::schematic {

    enum class ShaderStage : uint32_t {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute,
        Task,
        Mesh,
    };

    struct Shader final {
        ShaderStage stage;
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
    struct adl_serializer<Shader> {
        static auto to_json(json &j, const Shader &shader) {
            MU_CORE_ASSERT(shader.IsValid(), "shader must be valid in order to serialise");

            j["stage"] = static_cast<uint32_t>(shader.stage);

            if (shader.path) {
                j["path"] = shader.path->string();
            } else if (shader.byteOffset && shader.byteLength) {
                j["byteOffset"] = *shader.byteOffset;
                j["byteLength"] = *shader.byteLength;
            }

            j["entryPoint"] = shader.entryPoint;

            if (shader.workGroupSize) {
                j["workGroupSize"][0] = shader.workGroupSize->x;
                j["workGroupSize"][1] = shader.workGroupSize->y;
                j["workGroupSize"][2] = shader.workGroupSize->z;
            }
        }

        static auto from_json(const json &j, Shader &shader) {
            shader.stage = j["stage"].get<ShaderStage>();

            if (j.contains("path")) {
                shader.path = { j["path"].get<std::filesystem::path>() };
            } else if (j.contains("byteOffset") && j.contains("byteLength")) {
                shader.byteOffset = { j["byteOffset"].get<uint64_t>() };
                shader.byteLength = { j["byteLength"].get<uint64_t>() };
            } else {
                MU_CORE_ASSERT("path or binary information is missing");
            }

            shader.entryPoint = j["entryPoint"];

            if (j.contains("workGroupSize") && j["workGroupSize"].is_array() && j["workGroupSize"].size() == 3) {
                shader.workGroupSize = {
                    j["workGroupSize"][0].get<uint32_t>(),
                    j["workGroupSize"][1].get<uint32_t>(),
                    j["workGroupSize"][2].get<uint32_t>(),
                };
            }
        }
    };

}
