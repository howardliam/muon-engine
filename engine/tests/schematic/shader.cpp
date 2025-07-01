#include "muon/schematic/shader.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::schematic {

    using json = nlohmann::json;
    using namespace nlohmann::literals;

    const json shaderJsonWithPath = R"({
      "stage": 5,
      "entryPoint": "main",
      "path": "foo",
      "workGroupSize": [32, 32, 1]
    })"_json;

    TEST_CASE("shader with path serialization", "[schematic]") {
        Shader shader{};
        shader.stage = ShaderStage::Compute;
        shader.entryPoint = "main";
        shader.path = "foo";
        shader.workGroupSize = {32, 32, 1};

        const json j = shader;
        REQUIRE(j == shaderJsonWithPath);
    }

    TEST_CASE("shader with path deserialization", "[schematic]") {
        const auto shader = shaderJsonWithPath.get<Shader>();
        REQUIRE(shader.IsValid());
        REQUIRE(shader.stage == ShaderStage::Compute);
        REQUIRE(shader.entryPoint == "main");
        REQUIRE(shader.path == "foo");
        REQUIRE(shader.workGroupSize == glm::uvec3{32, 32, 1});
    }

    const json shaderJsonWithBinaryBlob = R"({
      "stage": 5,
      "entryPoint": "main",
      "byteOffset": 0,
      "byteLength": 120,
      "workGroupSize": [32, 32, 1]
    })"_json;

    TEST_CASE("shader with binary blob serialization", "[schematic]") {
        Shader shader{};
        shader.stage = ShaderStage::Compute;
        shader.entryPoint = "main";
        shader.byteOffset = 0;
        shader.byteLength = 120;
        shader.workGroupSize = {32, 32, 1};

        const json j = shader;
        REQUIRE(j == shaderJsonWithBinaryBlob);
    }

    TEST_CASE("shader with binary blob deserialization", "[schematic]") {
        const auto shader = shaderJsonWithBinaryBlob.get<Shader>();
        REQUIRE(shader.IsValid());
        REQUIRE(shader.stage == ShaderStage::Compute);
        REQUIRE(shader.entryPoint == "main");
        REQUIRE(shader.byteOffset == 0);
        REQUIRE(shader.byteLength == 120);
        REQUIRE(shader.workGroupSize == glm::uvec3{32, 32, 1});
    }

}
