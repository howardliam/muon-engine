#pragma once

#include "muon/schematic/pipeline_state_info.hpp"
#include "muon/schematic/shader.hpp"
#include <cstdint>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    enum class PipelineType : uint32_t {
        Graphics,
        Compute,
        Meshlet,
    };

    struct Pipeline {
        PipelineType type;
        std::optional<PipelineStateInfo> state{std::nullopt}; // used by graphics and meshlet pipelines
        std::unordered_map<ShaderStage, Shader> shaders{};

        [[nodiscard]] auto IsValid() const -> bool;
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<Pipeline> {
        static auto to_json(json &j, const auto &pipeline) {
            j["type"] = magic_enum::enum_name(pipeline.type);

            for (const auto &[stage, shader] : pipeline.shaders) {
                j["shaders"][magic_enum::enum_name(stage)] = shader;
            }

            if (pipeline.type != PipelineType::Compute) {
                j["state"] = pipeline.state;
            }
        }

        static auto from_json(const json &j, auto &pipeline) {
            pipeline.type = *magic_enum::enum_cast<PipelineType>(j["type"].get<uint32_t>());

            if (j["shaders"].is_object()) {
                for (const auto &[stage, shader] : j["shaders"].items()) {
                    auto stageKey = magic_enum::enum_cast<ShaderStage>(stage);
                    if (!stageKey.has_value()) { continue; }
                    pipeline.shaders[*stageKey] = shader.get<Shader>();
                }
            }

            if (pipeline.type != PipelineType::Compute && j.contains("state")) {
                pipeline.state = j["state"].get<PipelineStateInfo>();
            }
        }
    };

}
