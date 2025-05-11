#include "muon/engine/shader.hpp"

#include "muon/log/logger.hpp"
#include <cassert>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

namespace muon::engine {

    std::vector<uint8_t> readShaderFile(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error(std::format("failed to open file: {}", path.string()));
        }

        std::vector<uint8_t> buffer(file.tellg());

        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    EShLanguage shaderStageFromFilename(const std::string &filename) {
        if (filename.ends_with(".vert")) {
            return EShLangVertex;
        } else if (filename.ends_with(".tesc")) {
            return EShLangTessControl;
        } else if (filename.ends_with(".tese")) {
            return EShLangTessEvaluation;
        } else if (filename.ends_with(".geom")) {
            return EShLangGeometry;
        } else if (filename.ends_with(".frag")) {
            return EShLangFragment;
        } else if (filename.ends_with(".comp")) {
            return EShLangCompute;
        }

        else if (filename.ends_with(".vs.hlsl")) {
            return EShLangVertex;
        } else if (filename.ends_with(".ps.hlsl")) {
            return EShLangFragment;
        } else if (filename.ends_with(".cs.hlsl")) {
            return EShLangCompute;
        }  else if (filename.ends_with(".gs.hlsl")) {
            return EShLangGeometry;
        }  else if (filename.ends_with(".hs.hlsl")) {
            return EShLangTessControl;
        }  else if (filename.ends_with(".ds.hlsl")) {
            return EShLangTessEvaluation;
        }

        throw std::runtime_error("tried to parse shader stage from unknown format");
    }

    void compileShaders(const std::filesystem::path &directory) {
        assert(std::filesystem::is_directory(directory) && "must be a directory");

        std::vector<std::filesystem::path> glslFiles{};
        std::vector<std::filesystem::path> hlslFiles{};

        for (const auto& file : std::filesystem::directory_iterator(directory)) {
            const auto filename = file.path().filename().string();

            if (filename.ends_with("vert")) {
                glslFiles.push_back(file);
            } else if (filename.ends_with("tesc")) {
                glslFiles.push_back(file);
            } else if (filename.ends_with("tese")) {
                glslFiles.push_back(file);
            } else if (filename.ends_with("geom")) {
                glslFiles.push_back(file);
            } else if (filename.ends_with("frag")) {
                glslFiles.push_back(file);
            } else if (filename.ends_with("comp")) {
                glslFiles.push_back(file);
            }

            else if (filename.ends_with("vs.hlsl")) {
                hlslFiles.push_back(file);
            } else if (filename.ends_with("ps.hlsl")) {
                hlslFiles.push_back(file);
            } else if (filename.ends_with("cs.hlsl")) {
                hlslFiles.push_back(file);
            }  else if (filename.ends_with("gs.hlsl")) {
                hlslFiles.push_back(file);
            }  else if (filename.ends_with("hs.hlsl")) {
                hlslFiles.push_back(file);
            }  else if (filename.ends_with("ds.hlsl")) {
                hlslFiles.push_back(file);
            }
        }

        if (glslFiles.empty() && hlslFiles.empty()) {
            return;
        }

        glslang::InitializeProcess();

        for (const auto &path : glslFiles) {
            log::globalLogger->debug("compiling {}", path.string());

            auto data = readShaderFile(path);
            auto stage = shaderStageFromFilename(path.filename());
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

            auto newPath = path.string().append(".2.spv");
            std::ofstream outFile{newPath};
            outFile.write(reinterpret_cast<char *>(spirv.data()), spirv.size() * sizeof(uint32_t));
        }

        for (const auto &path : hlslFiles) {
            log::globalLogger->debug("compiling {}", path.string());

            auto data = readShaderFile(path);
            auto stage = shaderStageFromFilename(path.filename());
            glslang::TShader shader(stage);

            std::string source(reinterpret_cast<const char*>(data.data()), data.size());
            const char *strings[] = { source.c_str() };
            shader.setStrings(strings, 1);

            shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 100);
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

            auto newPath = path.string().append(".2.spv");
            std::ofstream outFile{newPath};
            outFile.write(reinterpret_cast<char *>(spirv.data()), spirv.size() * sizeof(uint32_t));
        }

        glslang::FinalizeProcess();
    }

}
