#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace mu::common::fs {

    /**
     * @brief   Compresses the file at the path.
     *
     * Compresses entire contents of files, writes it back into file and renames it with .zst extension.
     *
     * @param   path                the path of the file to be compressed.
     * @param   compressionLevel    the compression ratio, 1-22, defaults to 3.
     *
     * @return  whether it was successful, i.e.: attempting to compress a directory will return false.
    */
    bool compressFile(std::filesystem::path &path, int32_t compressionLevel = 3);

    /**
     * @brief   Reads the file at the path into a char vector.
     *
     * @param   path    the path of the file to be compressed.
     *
     * @return  optional, containing the char vector if successful.
    */
    std::optional<std::vector<char>> readFile(const std::filesystem::path &path);
}
