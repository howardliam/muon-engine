#include "muon/engine/shader.hpp"

#include "muon/log/logger.hpp"
#include <cassert>
#include <vector>

namespace muon::engine {

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

        for (const auto &file : glslFiles) {
            log::globalLogger->info("GLSL file: {}", file.filename().string());
        }

        for (const auto &file : hlslFiles) {
            log::globalLogger->info("HLSL file: {}", file.filename().string());
        }
    }

}
