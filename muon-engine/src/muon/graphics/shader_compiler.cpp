#include "muon/graphics/shader_compiler.hpp"

#include "SPIRV/GlslangToSpv.h"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/crypto/hash.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <mutex>
#include <thread>

namespace {

const std::unordered_map<std::string, EShLanguage> k_extensions{
    {".vert",         EShLanguage::EShLangVertex},
    {".tesc",    EShLanguage::EShLangTessControl},
    {".tese", EShLanguage::EShLangTessEvaluation},
    {".geom",       EShLanguage::EShLangGeometry},
    {".frag",       EShLanguage::EShLangFragment},
    {".task",           EShLanguage::EShLangTask},
    {".mesh",           EShLanguage::EShLangMesh},
    {".comp",        EShLanguage::EShLangCompute},
};

const TBuiltInResource k_defaultTBuiltInResource = {
    .maxLights = 32,
    .maxClipPlanes = 6,
    .maxTextureUnits = 32,
    .maxTextureCoords = 32,
    .maxVertexAttribs = 64,
    .maxVertexUniformComponents = 4096,
    .maxVaryingFloats = 64,
    .maxVertexTextureImageUnits = 32,
    .maxCombinedTextureImageUnits = 80,
    .maxTextureImageUnits = 32,
    .maxFragmentUniformComponents = 4096,
    .maxDrawBuffers = 32,
    .maxVertexUniformVectors = 128,
    .maxVaryingVectors = 8,
    .maxFragmentUniformVectors = 16,
    .maxVertexOutputVectors = 16,
    .maxFragmentInputVectors = 15,
    .minProgramTexelOffset = -8,
    .maxProgramTexelOffset = 7,
    .maxClipDistances = 8,
    .maxComputeWorkGroupCountX = 65535,
    .maxComputeWorkGroupCountY = 65535,
    .maxComputeWorkGroupCountZ = 65535,
    .maxComputeWorkGroupSizeX = 1024,
    .maxComputeWorkGroupSizeY = 1024,
    .maxComputeWorkGroupSizeZ = 64,
    .maxComputeUniformComponents = 1024,
    .maxComputeTextureImageUnits = 16,
    .maxComputeImageUniforms = 8,
    .maxComputeAtomicCounters = 8,
    .maxComputeAtomicCounterBuffers = 1,
    .maxVaryingComponents = 60,
    .maxVertexOutputComponents = 64,
    .maxGeometryInputComponents = 64,
    .maxGeometryOutputComponents = 128,
    .maxFragmentInputComponents = 128,
    .maxImageUnits = 8,
    .maxCombinedImageUnitsAndFragmentOutputs = 8,
    .maxCombinedShaderOutputResources = 8,
    .maxImageSamples = 0,
    .maxVertexImageUniforms = 0,
    .maxTessControlImageUniforms = 0,
    .maxTessEvaluationImageUniforms = 0,
    .maxGeometryImageUniforms = 0,
    .maxFragmentImageUniforms = 8,
    .maxCombinedImageUniforms = 8,
    .maxGeometryTextureImageUnits = 16,
    .maxGeometryOutputVertices = 256,
    .maxGeometryTotalOutputComponents = 1024,
    .maxGeometryUniformComponents = 1024,
    .maxGeometryVaryingComponents = 64,
    .maxTessControlInputComponents = 128,
    .maxTessControlOutputComponents = 128,
    .maxTessControlTextureImageUnits = 16,
    .maxTessControlUniformComponents = 1024,
    .maxTessControlTotalOutputComponents = 4096,
    .maxTessEvaluationInputComponents = 128,
    .maxTessEvaluationOutputComponents = 128,
    .maxTessEvaluationTextureImageUnits = 16,
    .maxTessEvaluationUniformComponents = 1024,
    .maxTessPatchComponents = 120,
    .maxPatchVertices = 32,
    .maxTessGenLevel = 64,
    .maxViewports = 16,
    .maxVertexAtomicCounters = 0,
    .maxTessControlAtomicCounters = 0,
    .maxTessEvaluationAtomicCounters = 0,
    .maxGeometryAtomicCounters = 0,
    .maxFragmentAtomicCounters = 8,
    .maxCombinedAtomicCounters = 8,
    .maxAtomicCounterBindings = 1,
    .maxVertexAtomicCounterBuffers = 0,
    .maxTessControlAtomicCounterBuffers = 0,
    .maxTessEvaluationAtomicCounterBuffers = 0,
    .maxGeometryAtomicCounterBuffers = 0,
    .maxFragmentAtomicCounterBuffers = 1,
    .maxCombinedAtomicCounterBuffers = 1,
    .maxAtomicCounterBufferSize = 16384,
    .maxTransformFeedbackBuffers = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances = 8,
    .maxCombinedClipAndCullDistances = 8,
    .maxSamples = 4,
    .maxMeshOutputVerticesNV = 256,
    .maxMeshOutputPrimitivesNV = 512,
    .maxMeshWorkGroupSizeX_NV = 32,
    .maxMeshWorkGroupSizeY_NV = 1,
    .maxMeshWorkGroupSizeZ_NV = 1,
    .maxTaskWorkGroupSizeX_NV = 32,
    .maxTaskWorkGroupSizeY_NV = 1,
    .maxTaskWorkGroupSizeZ_NV = 1,
    .maxMeshViewCountNV = 4,
    .maxMeshOutputVerticesEXT = 256,
    .maxMeshOutputPrimitivesEXT = 256,
    .maxMeshWorkGroupSizeX_EXT = 128,
    .maxMeshWorkGroupSizeY_EXT = 128,
    .maxMeshWorkGroupSizeZ_EXT = 128,
    .maxTaskWorkGroupSizeX_EXT = 128,
    .maxTaskWorkGroupSizeY_EXT = 128,
    .maxTaskWorkGroupSizeZ_EXT = 128,
    .maxMeshViewCountEXT = 4,
    .maxDualSourceDrawBuffersEXT = 1,

    .limits = {
               .nonInductiveForLoops = 1,
               .whileLoops = 1,
               .doWhileLoops = 1,
               .generalUniformIndexing = 1,
               .generalAttributeMatrixVectorIndexing = 1,
               .generalVaryingIndexing = 1,
               .generalSamplerIndexing = 1,
               .generalVariableIndexing = 1,
               .generalConstantMatrixVectorIndexing = 1,
               }
};

} // namespace

namespace muon::graphics {

ShaderCompiler::ShaderCompiler(const Spec &spec) : m_hashStore(spec.hashStorePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    bool success = glslang::InitializeProcess();
    core::expect(success, "failed to initialise shader compiler");

    m_hashStore.exec(R"(
        create table if not exists hash_store (
            id integer primary key autoincrement,
            source_path text,
            source_hash blob(32),
            spirv_path text
        )
    )");

    m_worker = std::thread([this]() {
        core::debug("shader compilation worker thread spawned");

        while (true) {
            core::trace("waiting for work");
            std::unique_lock<std::mutex> lock{m_workMutex};
            m_conVar.wait(lock, [this]() { return !m_workQueue.empty() || m_terminate.load(); });

            if (m_terminate.load()) {
                core::trace("terminate received");
                break;
            }

            auto request = m_workQueue.front();
            m_workQueue.pop();

            compile(request);
        }

        core::debug("shader compilation worker thread done");
    });

    core::debug("created shader compiler");
}

ShaderCompiler::~ShaderCompiler() {
    m_terminate.store(true);
    m_conVar.notify_one();
    m_worker.join();

    glslang::FinalizeProcess();
    core::debug("destroyed shader compiler");
}

auto ShaderCompiler::submitWork(ShaderCompilationRequest request) -> void {
    std::unique_lock<std::mutex> lock{m_workMutex};
    m_workQueue.push(request);
    m_conVar.notify_one();
}

auto ShaderCompiler::compile(const ShaderCompilationRequest &request) -> void {
    core::trace("beginning compilation");

    std::ifstream file{request.path, std::ios::binary};
    if (!file.is_open()) {
        core::error("failed to open file: {}", request.path.string());
        return;
    }

    std::array<uint8_t, 32> hash;

    SQLite::Statement readQuery{m_hashStore, R"(
        select * from hash_store where source_path = :path
    )"};
    readQuery.bind(":path", request.path.string());
    while (readQuery.executeStep()) {
        const uint8_t *rawHash = static_cast<const uint8_t *>(readQuery.getColumn(2).getBlob());
        std::copy(rawHash, rawHash + 32, hash.begin());
        break;
    }
    readQuery.reset();

    std::expected sourceHash = crypto::hashFile<32>(file);
    if (!sourceHash.has_value()) {
        core::error("failed to hash file contents: {}", request.path.string());
        return;
    }

    if (hash == *sourceHash) {
        core::trace("identical hashes, skipping: {}", request.path.string());
        return;
    }

    auto it = k_extensions.find(request.path.extension().string());
    if (it == k_extensions.end()) {
        core::error("failed to parse file extension: {}", request.path.extension().string());
        return;
    }
    auto [extension, stage] = *it;

    file.seekg(0, std::ios::end);
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), buffer.size());

    std::vector<const char *> shaderSrc = {buffer.data()};

    glslang::TShader shader{stage};
    shader.setStrings(shaderSrc.data(), shaderSrc.size());
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, 450);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);

    auto messages = static_cast<EShMessages>(EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules);

    auto success = shader.parse(&k_defaultTBuiltInResource, 450, false, messages);
    if (!success) {
        core::error("failed to parse GLSL source: \n{}", shader.getInfoLog());
        return;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    success = program.link(messages);
    if (!success) {
        core::error("failed to link shader program: \n{}", program.getInfoLog());
        return;
    }

    std::vector<uint32_t> spirv;

    glslang::SpvOptions options;
    options.generateDebugInfo = true;
    options.disableOptimizer = false;
    options.optimizeSize = true;

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &options);

    auto outPath = request.path.string();
    outPath.append(".spv");
    std::ofstream outFile{outPath, std::ios::binary};
    outFile.write(reinterpret_cast<char *>(spirv.data()), spirv.size() * sizeof(uint32_t));
    core::debug("writing out SPIR-V to {}", outPath);

    SQLite::Statement writeQuery{m_hashStore, R"(
        insert into hash_store values(null, :source_path, :source_hash, :spirv_path)
    )"};
    writeQuery.bind(":source_path", request.path.string());
    writeQuery.bind(":source_hash", sourceHash->data(), sourceHash->size());
    writeQuery.bind(":spirv_path", outPath.c_str());

    uint32_t rows = writeQuery.exec();
    core::trace("updated {} rows", rows);

    writeQuery.reset();
}

} // namespace muon::graphics
