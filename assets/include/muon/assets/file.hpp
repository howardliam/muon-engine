#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace muon::assets {

    std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path);

}
