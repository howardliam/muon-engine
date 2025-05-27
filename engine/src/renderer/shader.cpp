#include "muon/renderer/shader.hpp"

#include "muon/core/log.hpp"

#include <filesystem>
#include <vector>
#include <fstream>
#include <stdexcept>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

namespace muon {

    ShaderCompiler::ShaderCompiler() {
        glslang::InitializeProcess();
    }

    ShaderCompiler::~ShaderCompiler() {
        glslang::FinalizeProcess();
    }

    void ShaderCompiler::addShader(const std::filesystem::path &file) {
        if (!std::filesystem::is_regular_file(file)) {
            return;
        }

        const auto filename = file.filename().string();

        ShaderMetadata metadata;

        if (filename.ends_with(".vert")) {
            metadata.stage = ShaderStage::Vertex;
        } else if (filename.ends_with(".tesc")) {
            metadata.stage = ShaderStage::TessellationControl;
        } else if (filename.ends_with(".tese")) {
            metadata.stage = ShaderStage::TessellationEvaluation;
        } else if (filename.ends_with(".geom")) {
            metadata.stage = ShaderStage::Geometry;
        } else if (filename.ends_with(".frag")) {
            metadata.stage = ShaderStage::Fragment;
        } else if (filename.ends_with(".comp")) {
            metadata.stage = ShaderStage::Compute;
        } else {
            if (filename.ends_with(".spv")) {
                return;
            }
            MU_CORE_WARN("skipping unknown file: {}", filename);
            return;
        }

        metadata.language = ShaderLanguage::Glsl;

        shaders[file] = metadata;
        MU_CORE_DEBUG("added {} to compilation queue", filename);
    }

    void ShaderCompiler::addShaders(const std::filesystem::path &directory) {
        if (!std::filesystem::is_directory(directory)) {
            return;
        }

        for (const auto& file : std::filesystem::directory_iterator(directory)) {
            addShader(file.path());
        }
    }

    void ShaderCompiler::compile() {
        MU_CORE_DEBUG("beginning compilation of {} shaders", shaders.size());

        compileGlslToSpv();
    }

    std::vector<uint8_t> ShaderCompiler::readFile(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error(std::format("failed to open file: {}", path.string()));
        }

        std::vector<uint8_t> buffer(file.tellg());

        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    void ShaderCompiler::compileGlslToSpv() {
        std::vector<std::pair<std::filesystem::path, ShaderMetadata>> glslShaders;

        std::copy_if(
            shaders.begin(),
            shaders.end(),
            std::back_inserter(glslShaders),
            [](const auto &pair) {
                return pair.second.language == ShaderLanguage::Glsl;
            }
        );

        auto parseStage = [](const ShaderCompiler::ShaderStage &stage) {
            switch (stage) {
            case ShaderCompiler::ShaderStage::Vertex:
                return EShLangVertex;
            case ShaderCompiler::ShaderStage::TessellationControl:
                return EShLangTessControl;
            case ShaderCompiler::ShaderStage::TessellationEvaluation:
                return EShLangTessEvaluation;
            case ShaderCompiler::ShaderStage::Geometry:
                return EShLangGeometry;
            case ShaderCompiler::ShaderStage::Fragment:
                return EShLangFragment;
            case ShaderCompiler::ShaderStage::Compute:
                return EShLangCompute;
            }

            throw std::runtime_error("tried to parse shader stage from unknown format");
        };

        for (const auto &[path, metadata] : glslShaders) {
            MU_CORE_DEBUG("compiling {}", path.string());

            auto data = readFile(path);
            auto stage = parseStage(metadata.stage);
            glslang::TShader shader(stage);

            std::string source(reinterpret_cast<const char*>(data.data()), data.size());
            const char *strings[] = { source.c_str() };
            shader.setStrings(strings, 1);

            shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);

            EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

            TBuiltInResource resource{};
            resource.maxDrawBuffers = 1;
            resource.maxComputeWorkGroupSizeX = 64;
            resource.maxComputeWorkGroupSizeY = 64;
            resource.maxComputeWorkGroupSizeZ = 64;

            if (!shader.parse(&resource, 100, false, messages)) {
                throw std::runtime_error("failed to parse: " + path.string() + ", with reason: " + shader.getInfoLog());
            }

            glslang::TProgram program;
            program.addShader(&shader);
            if (!program.link(messages)) {
                throw std::runtime_error("failed to link: " + path.string() + ", with reason: " + program.getInfoLog());
            }

            std::vector<uint32_t> spirv;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

            auto newPath = path.string().append(".spv");
            std::ofstream outFile{newPath};
            outFile.write(reinterpret_cast<char *>(spirv.data()), spirv.size() * sizeof(uint32_t));
        }
    }

}
