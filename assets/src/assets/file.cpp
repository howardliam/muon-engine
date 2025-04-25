#include "muon/assets/file.hpp"

#include <fstream>

namespace muon::assets {

    std::optional<std::vector<uint8_t>> readFile(const std::filesystem::path &path) {
        if (!std::filesystem::is_regular_file(path)) {
            return {};
        }

        std::ifstream file{path, std::ios::binary};
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

}
