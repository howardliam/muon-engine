#include "muon/utils/fs.hpp"

#include "muon/core/log.hpp"
#include <fstream>

namespace muon::fs {

    std::vector<uint8_t> ReadFile(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate};

        if (!file.is_open()) {
            MU_CORE_ERROR("failed to open file for reading: {}", path.string());
            return {};
        }

        std::vector<uint8_t> buffer(file.tellg());
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    std::vector<uint8_t> ReadFileBinary(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            MU_CORE_ERROR("failed to open file for reading: {}", path.string());
            return {};
        }

        std::vector<uint8_t> buffer(file.tellg());
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

}
