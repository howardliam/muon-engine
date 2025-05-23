#include "muon/common/compress.hpp"

#include <cassert>
#include <zstd.h>

namespace mu::common::compress {

    std::optional<std::vector<char>> compressBuffer(std::vector<char> &buffer, int32_t compressionLevel) {
        size_t compressedSize = ZSTD_compressBound(buffer.size());
        std::vector<char> compressedBuffer(compressedSize);

        size_t result = ZSTD_compress(
            compressedBuffer.data(),
            compressedBuffer.size(),
            buffer.data(),
            buffer.size(),
            compressionLevel
        );

        if (ZSTD_isError(result)) {
           return {};
        }

        compressedBuffer.resize(result);

        return compressedBuffer;
    }

}
