#include "compression.hpp"

#include <cstdint>
#include <fstream>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <vector>
#include <zstd.h>

#include "core/exceptions.hpp"

namespace muon::compression {

    std::vector<char> compressBuffer(std::vector<char> &buffer) {
        size_t compressedSize = ZSTD_compressBound(buffer.size());
        std::vector<char> compressedBuffer(compressedSize);
        size_t compressedBytes = ZSTD_compress(
            compressedBuffer.data(),
            compressedBuffer.size(),
            buffer.data(),
            buffer.size(),
            1
        );

        if (ZSTD_isError(compressedBytes)) {
            throw except::CompressionFailedException(std::format("Compression failed: {}", ZSTD_getErrorName(compressedBytes)));
        }

        compressedBuffer.resize(compressedBytes);

        return compressedBuffer;
    }

    void compressFile(std::ifstream &inputStream, std::ofstream &outputStream) {
        inputStream.seekg(0, std::ios::end);
        int64_t size = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);

        std::vector<char> inputBuffer((static_cast<size_t>(size)));
        inputStream.read(inputBuffer.data(), static_cast<int64_t>(inputBuffer.size()));

        std::vector compressedBuffer = compressBuffer(inputBuffer);

        outputStream.write(compressedBuffer.data(), static_cast<int64_t>(compressedBuffer.size()));
    }

}
