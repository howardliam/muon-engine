#include "muon/graphics/shader_reflection.hpp"

#include "muon/core/assert.hpp"

#include <spirv_reflect.h>

namespace muon::graphics {

ShaderReflection::ShaderReflection(const ShaderReflectionSpecification &spec) {
    auto result =
        spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, spec.byteCode->size(), spec.byteCode->data(), &m_module);
    MU_CORE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);
}

ShaderReflection::~ShaderReflection() { spvReflectDestroyShaderModule(&m_module); }

std::unordered_map<std::string, glm::uvec3> ShaderReflection::GetWorkGroupSizes() const {
    std::unordered_map<std::string, glm::uvec3> workGroupSizes{};

    for (uint32_t i = 0; i < m_module.entry_point_count; i++) {
        auto &entryPoint = m_module.entry_points[i];

        workGroupSizes[entryPoint.name] = {entryPoint.local_size.x, entryPoint.local_size.y, entryPoint.local_size.z};
    }

    return workGroupSizes;
}

} // namespace muon::graphics
