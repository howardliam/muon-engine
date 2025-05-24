#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>

namespace mu {

    class ShaderCompiler {
    public:
        enum class ShaderLanguage;
        enum class ShaderStage;
        struct ShaderMetadata;

        ShaderCompiler();
        ~ShaderCompiler();

        void addShader(const std::filesystem::path &file);
        void addShaders(const std::filesystem::path &directory);

        void compile();

        void clearMemory();

        static std::vector<uint8_t> readFile(const std::filesystem::path &path);

    private:
        std::unordered_map<std::filesystem::path, ShaderMetadata> shaders;

        void compileGlslToSpv();
    };

    enum class ShaderCompiler::ShaderLanguage {
        Glsl,
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
