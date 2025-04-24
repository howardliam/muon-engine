#include "muon/engine/pipeline.hpp"

#include <format>
#include <fstream>
#include <stdexcept>

namespace muon::engine {

    std::vector<uint8_t> readShaderFile(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error(std::format("failed to open file: {}", path.string()));
        }

        std::vector<uint8_t> buffer(file.tellg());

        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

}
