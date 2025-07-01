#pragma once

#include "muon/core/assert.hpp"
#include "muon/schematic/pipeline_state.hpp"
#include "muon/schematic/shader.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <map>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <string>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    enum class PipelineType : uint32_t {
        Graphics,
        Compute,
        Meshlet,
    };

    struct Pipeline {
        PipelineType type;
        std::optional<PipelineState> state{std::nullopt}; // used by graphics and meshlet pipelines
        std::map<ShaderStage, Shader> shaders{};

        [[nodiscard]] auto IsValid() const -> bool;
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<Pipeline> {
        static auto to_json(json &j, const Pipeline &pipeline) {
            j["type"] = static_cast<uint32_t>(pipeline.type);

            for (const auto &[stage, shader] : pipeline.shaders) {
                j["shaders"][fmt::format("{}", static_cast<uint32_t>(stage))] = shader;
            }
        }

        static auto from_json(const json &j, Pipeline &pipeline) {
            pipeline.type = j["type"].get<PipelineType>();

            MU_CORE_ASSERT(j["shaders"].is_object(), "shaders must be a map");
            for (const auto &[stage, shader] : j["shaders"].items()) {
                try {
                    pipeline.shaders[static_cast<ShaderStage>(std::stoi(stage))] = shader.get<Shader>();
                } catch (const std::exception &e) {
                    MU_CORE_ERROR("failed to parse {} to shader stage key");
                }
            }

            if (pipeline.type == PipelineType::Compute) { return; }
            MU_CORE_ASSERT(j.contains("state"), "state must exist on graphics and meshlet pipelines");

            auto state = j["state"];
        }
    };

}
