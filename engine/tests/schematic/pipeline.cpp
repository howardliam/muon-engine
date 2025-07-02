#include "muon/schematic/pipeline.hpp"
#include "muon/schematic/shader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

namespace muon::schematic {

    using json = nlohmann::json;
    using namespace nlohmann::literals;

    TEST_CASE("pipeline serialization", "[schematic]") {
        Pipeline pipeline{};
        pipeline.type = PipelineType::Compute;

        pipeline.shaders[ShaderStage::Task].entryPoint = "main";
        pipeline.shaders[ShaderStage::Task].path = "foo";
        pipeline.shaders[ShaderStage::Task].workGroupSize = std::nullopt;

        pipeline.shaders[ShaderStage::Mesh].entryPoint = "main";
        pipeline.shaders[ShaderStage::Mesh].path = "foo";
        pipeline.shaders[ShaderStage::Mesh].workGroupSize = {32, 32, 1};

        pipeline.shaders[ShaderStage::Fragment].entryPoint = "main";
        pipeline.shaders[ShaderStage::Fragment].path = "foo";
        pipeline.shaders[ShaderStage::Fragment].workGroupSize = std::nullopt;

        const json j = pipeline;

        std::cout << j.dump(4) << '\n';

    }

    TEST_CASE("pipeline deserialization", "[schematic]") {
    }

}
