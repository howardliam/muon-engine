#pragma once

#include "muon/schematic/pipeline/pipeline_state_info.hpp"
#include "muon/schematic/pipeline/shader_info.hpp"
#include <cstdint>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    enum class PipelineType {
        Graphics,
        Compute,
        Meshlet,
    };

    struct PipelineInfo {
        PipelineType type;
        std::optional<PipelineStateInfo> state{std::nullopt}; // used by graphics and meshlet pipelines
        std::unordered_map<VkShaderStageFlagBits, ShaderInfo> shaders{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<PipelineInfo> {
        static auto to_json(json &j, const auto &info) {
            j["type"] = magic_enum::enum_name(info.type);

            for (const auto &[stage, shader] : info.shaders) {
                j["shaders"][magic_enum::enum_name(stage)] = shader;
            }

            if (info.type != PipelineType::Compute) {
                j["state"] = info.state;
            }
        }

        static auto from_json(const json &j, auto &info) {
            info.type = *magic_enum::enum_cast<PipelineType>(j["type"].get<uint32_t>());

            if (j["shaders"].is_object()) {
                for (const auto &[stage, shader] : j["shaders"].items()) {
                    auto stageKey = magic_enum::enum_cast<VkShaderStageFlagBits>(stage);
                    if (!stageKey.has_value()) { continue; }
                    info.shaders[*stageKey] = shader.get<ShaderInfo>();
                }
            }

            if (info.type != PipelineType::Compute && j.contains("state")) {
                info.state = j["state"].get<PipelineStateInfo>();
            }
        }
    };

}
