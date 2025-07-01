#pragma once

#include "muon/core/assert.hpp"
#include "muon/schematic/shader.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vector>

namespace muon::schematic {

    enum class PipelineType : uint32_t {
        Graphics,
        Compute,
        Meshlet,
    };

    struct Pipeline {
        PipelineType type;
        std::vector<Shader> shaders{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<Pipeline> {
        static void to_json(json &j, const Pipeline &pipeline) {
            j["type"] = static_cast<uint32_t>(pipeline.type);

            for (const auto &shader : pipeline.shaders) {
                j["shaders"].push_back(shader);
            }
        }

        static void from_json(const json &j, Pipeline &pipeline) {
            pipeline.type = j["type"].get<PipelineType>();

            MU_CORE_ASSERT(j["shaders"].is_array(), "shaders must be an array");
            pipeline.shaders.resize(j["shaders"].size());
            for (uint32_t i = 0; i < j["shaders"].size(); i++) {
                pipeline.shaders[i] = j["shaders"][i].get<Shader>();
            }
        }
    };

}
