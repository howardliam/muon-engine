#pragma once

#include <bitset>
#include <cstdint>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct InputAssemblyState {
        VkPrimitiveTopology topology;
        bool primitiveRestartEnable{false};
    };

    struct ViewportState {
        uint32_t viewportCount{1};
        uint32_t scissorCount{1};
    };

    struct RasterizationState {
        bool depthClampEnable{false};
        bool rasterizerDiscardEnable{false};
        VkPolygonMode polygonMode;
        VkCullModeFlagBits cullMode;
        VkFrontFace frontFace;
        bool depthBiasEnable{false};
        float depthBiasConstantFactor{0.0};
        float depthBiasClamp{0.0};
        float depthBiasSlopeFactor{0.0};
        std::optional<float> lineWidth{std::nullopt};
    };

    struct MultisampleState {
        std::bitset<7> rasterizationSamples{0};
        bool sampleShadingEnable{false};
        std::optional<float> minSampleShading{std::nullopt};
        bool alphaToCoverageEnable{false};
        bool alphaToOneEnable{false};
    };

    struct ColorBlendAttachment {
        bool blendEnable{false};
        VkBlendFactor srcColorBlendFactor;
        VkBlendFactor dstColorBlendFactor;
        VkBlendOp colorBlendOp;
        VkBlendFactor srcAlphaBlendFactor;
        VkBlendFactor dstAlphaBlendFactor;
        VkBlendOp alphaBlendOp;
        VkColorComponentFlags colorWriteMask;
    };

    struct ColorBlendState {
        bool logicOpEnable{false};
        std::optional<VkLogicOp> logicOp{std::nullopt};
        std::vector<ColorBlendAttachment> attachments{};
        std::array<float, 4> blendConstants{0.0};
    };

    struct StencilOpState {
        VkStencilOp failOp;
        VkStencilOp passOp;
        VkStencilOp depthFailOp;
        VkCompareOp compareOp;
        uint32_t compareMask{0};
        uint32_t writeMask{0};
        uint32_t reference{0};
    };

    struct DepthStencilState {
        bool depthTestEnable{true};
        bool depthWriteEnable{true};
        VkCompareOp depthCompareOp;
        bool depthBoundsTestEnable{false};
        float minDepthBounds{0.0};
        float maxDepthBounds{1.0};
        bool stencilTestEnable{false};
        std::optional<StencilOpState> front{std::nullopt};
        std::optional<StencilOpState> back{std::nullopt};
    };

    struct DynamicState {
        std::vector<VkDynamicState> states{};
    };

    struct PipelineState {
        std::optional<InputAssemblyState> inputAssembly{}; // only used by graphics pipelines
        ViewportState viewport{};
        RasterizationState rasterization{};
        MultisampleState multisample{};
        ColorBlendState colorBlend{};
        DepthStencilState depthStencil{};
        DynamicState dynamic{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<InputAssemblyState> {
        static auto to_json(json &j, const InputAssemblyState &state) {

        }

        static auto from_json(const json &j, InputAssemblyState &state) {
            state.topology = j["topology"].get<VkPrimitiveTopology>();
            state.primitiveRestartEnable = j["primitiveRestartEnable"].get<bool>();
        }
    };

    template<>
    struct adl_serializer<PipelineState> {
        static auto to_json(json &j, const PipelineState &state) {

        }

        static auto from_json(const json &j, PipelineState &state) {

        }
    };

}
