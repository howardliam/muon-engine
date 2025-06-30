#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace muon::fs {

    [[nodiscard]] std::vector<uint8_t> ReadFile(const std::filesystem::path &path);
    [[nodiscard]] std::vector<uint8_t> ReadFileBinary(const std::filesystem::path &path);

}
