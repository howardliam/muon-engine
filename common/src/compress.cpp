#include "muon/common/compress.hpp"

#include <zstd.h>

namespace muon::common::compress {

    std::optional<std::vector<char>> compressBuffer(std::vector<char> &buffer) {
        size_t compressedSize = ZSTD_compressBound(buffer.size());
        std::vector<char> compressedBuffer(compressedSize);

        size_t result = ZSTD_compress(
            compressedBuffer.data(),
            compressedBuffer.size(),
            buffer.data(),
            buffer.size(),
            3
        );

        if (ZSTD_isError(result)) {
           return {};
        }

        return compressedBuffer;
    }

}
