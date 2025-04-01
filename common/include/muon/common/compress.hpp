#pragma once

#include <optional>
#include <vector>

namespace muon::common::compress {

    /**
     * @brief   Compresses the buffer using libzstd
     *
     * @param   buffer: the buffer to compress.
     *
     * @return  returns compressed buffer if successful, otherwise nothing.
    */
    std::optional<std::vector<char>> compressBuffer(std::vector<char> &buffer);

}
