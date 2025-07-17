#include "muon/graphics/shader_compiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <cstring>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <optional>
#include <string_view>

namespace {

constexpr auto Hash(const std::string_view string) -> uint64_t {
    uint64_t hash = 0;
    for (char c : string) {
        hash = (hash * 131) + c;
    }
    return hash;
}

constexpr auto operator""_hash(const char *string, size_t length) -> uint64_t { return Hash(std::string_view(string, length)); }

auto ExtensionToStage(const std::string_view extension) -> std::optional<EShLanguage> {
    switch (Hash(extension)) {
        case ".vert"_hash:
            return EShLanguage::EShLangVertex;

        case ".tesc"_hash:
            return EShLanguage::EShLangTessControl;

        case ".tese"_hash:
            return EShLanguage::EShLangTessEvaluation;

        case ".geom"_hash:
            return EShLanguage::EShLangGeometry;

        case ".frag"_hash:
            return EShLanguage::EShLangFragment;

        case ".task"_hash:
            return EShLanguage::EShLangTask;

        case ".mesh"_hash:
            return EShLanguage::EShLangMesh;

        case ".comp"_hash:
            return EShLanguage::EShLangCompute;

        default:
            return std::nullopt;
    }
}

} // namespace

namespace muon::graphics {

ShaderCompiler::ShaderCompiler(const Spec &spec) {
    bool success = glslang::InitializeProcess();
    MU_CORE_ASSERT(success, "failed to initialise shader compiler");

    std::memcpy(&m_resource, GetDefaultResources(), sizeof(TBuiltInResource));

    MU_CORE_DEBUG("created shader compiler");
}

ShaderCompiler::~ShaderCompiler() {
    glslang::FinalizeProcess();
    MU_CORE_DEBUG("destroyed shader compiler");
}

} // namespace muon::graphics
