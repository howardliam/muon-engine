#include "muon/schematic/pipeline.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::schematic {

    using json = nlohmann::json;
    using namespace nlohmann::literals;

    const auto jsonMeshletPipeline = R"({
        "shaders": {
            "Fragment": {
                "entryPoint": "main",
                "path": "foo"
            },
            "Mesh": {
                "entryPoint": "main",
                "path": "foo",
                "workGroupSize": [
                    32,
                    32,
                    1
                ]
            },
            "Task": {
                "entryPoint": "main",
                "path": "foo"
            }
        },
        "state": {
            "colorBlend": {
                "blendConstants": [
                    0.0,
                    0.0,
                    0.0,
                    0.0
                ],
                "logicOpEnable": false
            },
            "depthStencil": {
                "depthTestEnable": false,
                "stencilTestEnable": false
            },
            "dynamic": {
                "states": [
                    0,
                    1
                ]
            },
            "multisample": {
                "alphaToCoverageEnable": false,
                "alphaToOneEnable": false,
                "rasterizationSamples": 0,
                "sampleShadingEnable": false
            },
            "rasterization": {
                "cullMode": 2,
                "depthBiasClamp": 0.0,
                "depthBiasConstantFactor": 0.0,
                "depthBiasEnable": false,
                "depthBiasSlopeFactor": 0.0,
                "depthClampEnable": false,
                "frontFace": 1,
                "polygonMode": 0,
                "rasterizerDiscardEnable": false
            },
            "viewport": {
                "scissorCount": 1,
                "viewportCount": 1
            }
        },
        "type": "Meshlet"
    })"_json;

    TEST_CASE("pipeline serialization", "[schematic]") {
        Pipeline pipeline{};
        pipeline.type = PipelineType::Meshlet;

        pipeline.shaders[ShaderStage::Task].entryPoint = "main";
        pipeline.shaders[ShaderStage::Task].path = "foo";
        pipeline.shaders[ShaderStage::Task].workGroupSize = std::nullopt;

        pipeline.shaders[ShaderStage::Mesh].entryPoint = "main";
        pipeline.shaders[ShaderStage::Mesh].path = "foo";
        pipeline.shaders[ShaderStage::Mesh].workGroupSize = {32, 32, 1};

        pipeline.shaders[ShaderStage::Fragment].entryPoint = "main";
        pipeline.shaders[ShaderStage::Fragment].path = "foo";
        pipeline.shaders[ShaderStage::Fragment].workGroupSize = std::nullopt;

        pipeline.state.emplace();

        pipeline.state->viewport.viewportCount = 1;
        pipeline.state->viewport.scissorCount = 1;

        pipeline.state->rasterization.cullMode = CullMode::Back;
        pipeline.state->rasterization.polygonMode = PolygonMode::Fill;
        pipeline.state->rasterization.frontFace = FrontFace::Clockwise;

        pipeline.state->dynamic.states.push_back(DynamicState::Viewport);
        pipeline.state->dynamic.states.push_back(DynamicState::Scissor);

        const json j = pipeline;
        REQUIRE(j == jsonMeshletPipeline);
    }

    TEST_CASE("pipeline deserialization", "[schematic]") {

    }

}
