#pragma once

#include <filesystem>
#include <vector>

namespace muon::engine {

    std::vector<uint8_t> readShaderFile(const std::filesystem::path &path);

    void compileShaders(const std::filesystem::path &directory);

}
