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
        std::vector compressedBuffer = result.value();

        auto newPath = path.string() + "1.zst";
        std::ofstream outFile{newPath, std::ios::binary};
        outFile.write(compressedBuffer.data(), compressedBuffer.size());

        return true;
    }

}
