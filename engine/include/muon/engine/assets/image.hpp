#pragma once

#include <cstdint>
#include <vector>

namespace muon::engine::assets {

    struct ImageData {
        uint32_t width;
        uint32_t height;
        uint8_t bitDepth;
        std::vector<char> data;
    };

    ImageData loadImagePng(std::vector<char> imageData);

}
