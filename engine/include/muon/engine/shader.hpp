#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>

namespace muon::engine {

    std::vector<uint8_t> readShaderFile(const std::filesystem::path &path);

    void compileShaders(const std::filesystem::path &directory);

    class ShaderCompiler {
    public:
        enum class ShaderLanguage;
        enum class ShaderStage;
        struct ShaderMetadata;

        ShaderCompiler();
        ~ShaderCompiler();

        void addShaders(const std::filesystem::path &directory);

        void compile();

        void clearMemory();

        static std::vector<uint8_t> readFile(const std::filesystem::path &path);

    private:
        std::unordered_map<std::filesystem::path, ShaderMetadata> shaders;

        void compileGlslToSpv();
        void compileHlslToSpv();
    };

    enum class ShaderCompiler::ShaderLanguage {
        Glsl,
        Hlsl,
    };

    enum class ShaderCompiler::ShaderStage {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute,
    };

    struct ShaderCompiler::ShaderMetadata {
        ShaderLanguage language;
        ShaderStage stage;
    };

}
