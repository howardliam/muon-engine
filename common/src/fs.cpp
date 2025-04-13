#include "muon/common/fs.hpp"
#include "muon/common/compress.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace muon::common::fs {

    bool compressFile(std::filesystem::path &path, int32_t compressionLevel) {
        if (!std::filesystem::is_regular_file(path)) {
            return false;
        }

        std::ifstream inFile{path, std::ios::binary};
        inFile.seekg(0, std::ios::end);
        size_t size = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        std::vector<char> inputBuffer(size);
        inFile.read(inputBuffer.data(), inputBuffer.size());

        auto result = compress::compressBuffer(inputBuffer, compressionLevel);
        if (!result.has_value()) {
            return false;
        }

        auto newPath = path.string() + ".zst";
        std::ofstream outFile{newPath, std::ios::binary};
        outFile.write(result->data(), result->size());

        return true;
    }

    std::optional<std::vector<char>> readFile(const std::filesystem::path &path) {
        if (!std::filesystem::is_regular_file(path)) {
            return {};
        }

        std::ifstream file{path, std::ios::binary};
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        file.read(buffer.data(), buffer.size());
        return buffer;
    }

}
