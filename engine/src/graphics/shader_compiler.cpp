#include "muon/graphics/shader_compiler.hpp"

#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include "muon/core/assert.hpp"
#include "muon/core/hash.hpp"
#include "muon/core/log.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <glslang/Public/ShaderLang.h>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>

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

const TBuiltInResource k_defaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxMeshOutputVerticesEXT = */ 256,
    /* .maxMeshOutputPrimitivesEXT = */ 256,
    /* .maxMeshWorkGroupSizeX_EXT = */ 128,
    /* .maxMeshWorkGroupSizeY_EXT = */ 128,
    /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
    /* .maxTaskWorkGroupSizeX_EXT = */ 128,
    /* .maxTaskWorkGroupSizeY_EXT = */ 128,
    /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
    /* .maxMeshViewCountEXT = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */
    {
     /* .nonInductiveForLoops = */ 1,
     /* .whileLoops = */ 1,
     /* .doWhileLoops = */ 1,
     /* .generalUniformIndexing = */ 1,
     /* .generalAttributeMatrixVectorIndexing = */ 1,
     /* .generalVaryingIndexing = */ 1,
     /* .generalSamplerIndexing = */ 1,
     /* .generalVariableIndexing = */ 1,
     /* .generalConstantMatrixVectorIndexing = */ 1,
     }
};

} // namespace

namespace muon::graphics {

ShaderCompiler::ShaderCompiler(const Spec &spec) : m_hashStore(spec.hashStorePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    bool success = glslang::InitializeProcess();
    MU_CORE_ASSERT(success, "failed to initialise shader compiler");

    m_hashStore.exec(R"(
        create table if not exists hash_store (
            id integer primary key autoincrement,
            source_path text,
            source_hash blob(32),
            spirv_path text
        )
    )");

    m_resource = k_defaultTBuiltInResource;

    m_worker = std::thread([this]() {
        MU_CORE_DEBUG("shader compilation worker thread spawned");

        while (true) {
            MU_CORE_TRACE("checking for work");
            std::unique_lock<std::mutex> lock{m_workMutex};
            m_conVar.wait(lock, [this]() { return !m_workQueue.empty() || m_terminate.load(); });

            if (m_terminate.load()) {
                MU_CORE_TRACE("terminate received");
                break;
            }

            auto request = m_workQueue.front();
            m_workQueue.pop();

            Compile(request);
        }

        MU_CORE_DEBUG("shader compilation worker thread done");
    });

    MU_CORE_DEBUG("created shader compiler");
}

ShaderCompiler::~ShaderCompiler() {
    m_terminate.store(true);
    m_conVar.notify_one();
    m_worker.join();

    glslang::FinalizeProcess();
    MU_CORE_DEBUG("destroyed shader compiler");
}

auto ShaderCompiler::SubmitWork(ShaderCompilationRequest request) -> void {
    std::unique_lock<std::mutex> lock{m_workMutex};
    m_workQueue.push(request);
    m_conVar.notify_one();
}

auto ShaderCompiler::Compile(const ShaderCompilationRequest &request) -> void {
    MU_CORE_TRACE("beginning compilation");

    std::ifstream file{request.path, std::ios::binary};
    if (!file.is_open()) {
        MU_CORE_ERROR("failed to open file: {}", request.path.generic_string());
        return;
    }

    std::array<uint8_t, 32> hash;

    SQLite::Statement readQuery{m_hashStore, R"(
        select * from hash_store where source_path = :path
    )"};
    readQuery.bind(":path", request.path.c_str());
    while (readQuery.executeStep()) {
        const uint8_t *rawHash = static_cast<const uint8_t *>(readQuery.getColumn(2).getBlob());
        std::copy(rawHash, rawHash + 32, hash.begin());
        break;
    }
    readQuery.reset();

    std::optional sourceHash = HashFile(file);
    if (!sourceHash.has_value()) {
        MU_CORE_ERROR("failed to hash file contents: {}", request.path.extension().generic_string());
        return;
    }

    for (uint32_t i = 0; i < 32; i++) {
        MU_CORE_INFO("{}, {}", hash[i], (*sourceHash)[i]);
    }

    if (hash == *sourceHash) {
        MU_CORE_TRACE("identical hashes, skipping: {}", request.path.extension().generic_string());
        return;
    }

    auto stage = ExtensionToStage(request.path.extension().generic_string());
    if (!stage.has_value()) {
        MU_CORE_ERROR("failed to parse file extension: {}", request.path.extension().generic_string());
        return;
    }

    file.clear();
    file.seekg(0, std::ios::end);
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), buffer.size());

    std::vector<const char *> shaderSrc = {buffer.data()};

    glslang::TShader shader(*stage);
    shader.setStrings(shaderSrc.data(), shaderSrc.size());
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl, *stage, glslang::EShClient::EShClientVulkan, 450);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);

    auto messages = static_cast<EShMessages>(EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules);

    auto success = shader.parse(&m_resource, 450, false, messages);
    if (!success) {
        MU_CORE_ERROR("failed to parse GLSL source: \n{}", shader.getInfoLog());
        return;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    success = program.link(messages);
    if (!success) {
        MU_CORE_ERROR("failed to link shader program: \n{}", program.getInfoLog());
        return;
    }

    std::vector<uint32_t> spirv;

    glslang::SpvOptions options;
    options.generateDebugInfo = true;
    options.disableOptimizer = false;
    options.optimizeSize = true;

    glslang::GlslangToSpv(*program.getIntermediate(*stage), spirv, &options);

    auto outPath = request.path.string();
    outPath.append(".spv");
    std::ofstream outFile{outPath, std::ios::binary};
    outFile.write(reinterpret_cast<char *>(spirv.data()), spirv.size() * sizeof(uint32_t));
    MU_CORE_DEBUG("writing out SPIR-V to {}", outPath);

    SQLite::Statement writeQuery{m_hashStore, R"(
        insert into hash_store values(null, :source_path, :source_hash, :spirv_path)
    )"};
    writeQuery.bind(":source_path", request.path.c_str());
    writeQuery.bind(":source_hash", sourceHash->data(), sourceHash->size());
    writeQuery.bind(":spirv_path", outPath.c_str());

    uint32_t rows = writeQuery.exec();
    MU_CORE_TRACE("updated {} rows", rows);

    // writeQuery.reset();
}

} // namespace muon::graphics
