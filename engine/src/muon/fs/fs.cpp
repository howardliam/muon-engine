#include "muon/fs/fs.hpp"

#include "muon/core/log.hpp"

#include <fstream>
#include <sstream>

namespace muon::fs {

auto readFile(const std::filesystem::path &path) -> std::optional<std::string> {
    std::ifstream file{path};

    if (!file.is_open()) {
        core::error("failed to open file for reading: {}", path.string());
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

auto readFileBinary(const std::filesystem::path &path) -> std::optional<std::vector<uint8_t>> {
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        core::error("failed to open file for reading: {}", path.string());
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(file.tellg());
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    return buffer;
}

auto writeFile(const uint8_t *data, size_t size, const std::filesystem::path &path) -> bool {
    std::ofstream file{path, std::ios::binary};
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char *>(data), size);
    return true;
}

} // namespace muon::fs
