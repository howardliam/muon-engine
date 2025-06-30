#pragma once

#include <glm/vec3.hpp>
#include <spirv_reflect.h>
#include <unordered_map>
#include <vector>

namespace muon::graphics {

    struct ShaderReflectionSpecification {
        const std::vector<uint8_t> *byteCode;
    };

    class ShaderReflection {
    public:
        ShaderReflection(const ShaderReflectionSpecification &spec);
        ~ShaderReflection();

    public:
        [[nodiscard]] std::unordered_map<std::string, glm::uvec3> GetWorkGroupSizes() const;

    private:
        SpvReflectShaderModule m_module{};
    };

}
