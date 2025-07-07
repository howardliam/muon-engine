#include "muon/fs/fs.hpp"

#include "muon/core/log.hpp"
#include <fstream>
#include <sstream>

namespace muon::fs {

    auto ReadFile(const std::filesystem::path &path) -> std::optional<std::string> {
        std::ifstream file{path, std::ios::ate};

        if (!file.is_open()) {
            MU_CORE_ERROR("failed to open file for reading: {}", path.string());
            return std::nullopt;
        }

        std::stringstream buffer;
        file.seekg(0);
        buffer << file.rdbuf();

        return buffer.str();
    }

    auto ReadFileBinary(const std::filesystem::path &path) -> std::optional<std::vector<uint8_t>> {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            MU_CORE_ERROR("failed to open file for reading: {}", path.string());
            return std::nullopt;
        }

        std::vector<uint8_t> buffer(file.tellg());
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    auto WriteFile(const uint8_t *data, size_t size, const std::filesystem::path &path) -> bool {
        std::ofstream file{path, std::ios::binary};
        if (!file.is_open()) { return false; }

        file.write(reinterpret_cast<const char *>(data), size);
        return true;
    }

}
