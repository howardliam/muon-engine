#pragma once

#include <filesystem>

namespace muon::common::fs {

    /**
     * @brief   Compresses the file at the path.
     *
     * Compresses entire contents of files, writes it back into file and renames it with .zst extension.
     *
     * @param   path:   the path of the file to be compressed.
     *
     * @return  whether it was successful, i.e.: attempting to compress a directory will return false.
    */
    bool compressFile(std::filesystem::path &path);

}
