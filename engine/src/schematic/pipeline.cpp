#include "muon/schematic/pipeline.hpp"
#include <bitset>

namespace muon::schematic {

    constexpr std::bitset<8> k_graphicsPipelineShaderStages{0b01111100};
    constexpr std::bitset<8> k_graphicsPipelineRequiredStages{0b01000100};

    constexpr std::bitset<8> k_computePipelineShaderStages{0b10000000};
    constexpr std::bitset<8> k_computePipelineRequiredStages{0b10000000};

    constexpr std::bitset<8> k_meshletPipelineShaderStages{0b01000011};
    constexpr std::bitset<8> k_meshletPipelineRequiredStages{0b01000010};

    auto Pipeline::IsValid() const -> bool {
        std::bitset<8> shaderOccupancy{0};

        for (const auto &[stage, _] : shaders) {
            shaderOccupancy.set(static_cast<uint32_t>(stage));
        }

        bool correctShaders = false;
        bool requiredShaders = false;

        switch (type) {
            case PipelineType::Graphics: {
                correctShaders = (shaderOccupancy & k_graphicsPipelineShaderStages).any();
                requiredShaders = (shaderOccupancy & k_graphicsPipelineRequiredStages).any();
                break;
            }
            case PipelineType::Compute: {
                correctShaders = (shaderOccupancy & k_computePipelineShaderStages).any();
                requiredShaders = (shaderOccupancy & k_computePipelineRequiredStages).any();
                break;
            }
            case PipelineType::Meshlet: {
                correctShaders = (shaderOccupancy & k_meshletPipelineShaderStages).any();
                requiredShaders = (shaderOccupancy & k_meshletPipelineRequiredStages).any();
                break;
            }
        }

        if (!correctShaders) { return false; }
        if (!requiredShaders) { return false; }

        return true;
    }

}
