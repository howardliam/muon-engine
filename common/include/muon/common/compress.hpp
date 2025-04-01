#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace muon::common::compress {

    /**
     * @brief   Compresses the buffer using libzstd
     *
     * @param   buffer:             the buffer to compress.
     * @param   compressionLevel:    the compression ratio, 1-22, defaults to 3.
     *
     * @return  returns compressed buffer if successful, otherwise nothing.
    */
    std::optional<std::vector<char>> compressBuffer(std::vector<char> &buffer, int32_t compressionLevel = 3);

}
