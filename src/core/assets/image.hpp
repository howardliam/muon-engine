#pragma once

#include <filesystem>
#include <vector>

namespace muon::assets {

    struct PngProperties {
        uint32_t width;
        uint32_t height;
        int32_t color_type;
        int32_t bit_depth;
    };

    void readPngFile(std::filesystem::path path, std::vector<uint8_t> &imageData, PngProperties &properties);


}
