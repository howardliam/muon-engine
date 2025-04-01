#include "muon/common/fs.hpp"

namespace muon::common::fs {

    bool compressFile(std::filesystem::path path) {
        if (std::filesystem::is_directory(path)) {
            return false;
        }

        return true;
    }

}
