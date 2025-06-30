#include "muon/schematic/shader.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::schematic {
    using json = nlohmann::json;
    using namespace nlohmann::literals;

    const json testWithPath = R"({
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

        json j = shader;

        REQUIRE(j == testWithPath);
    }

    TEST_CASE("shader with path deserialization", "[schematic]") {
        auto shader = testWithPath.get<Shader>();
        REQUIRE(shader.stage == ShaderStage::Compute);
        REQUIRE(shader.entryPoint == "main");
        REQUIRE(shader.path == "foo");
        REQUIRE(shader.workGroupSize == glm::uvec3{32, 32, 1});
    }

}
