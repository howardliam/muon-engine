#pragma once

#include <filesystem>
#include <vector>

namespace muon::engine {

    std::vector<uint8_t> readShaderFile(const std::filesystem::path &path);

    void compileShaders(const std::filesystem::path &directory);

    class ShaderCompiler {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        void addShaders(const std::filesystem::path &directory);

        void compile();

        void clearMemory();

        static std::vector<uint8_t> readShaderFile(const std::filesystem::path &path);

    private:
        std::vector<std::filesystem::path> shaderPaths{};

        void compileGlslToSpv();
        void compileHlslToSpv();
    };

}
