#pragma once

#include <cstdint>
#include <vector>
#include "muon/asset/assetloader.hpp"

namespace muon::asset {

    struct ImageAsset : public Asset {
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        uint8_t bitDepth;
        std::vector<uint8_t> data;

        virtual AssetType type() const override {
            return AssetType::Image;
        }
    };

}
