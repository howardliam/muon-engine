#pragma once

#include "muon/asset/assetloader.hpp"
#include <cstdint>
#include <vector>

namespace muon::asset {

    struct Audio {
        uint32_t sampleRate;
        uint32_t channels;
        std::vector<float> samples{};
    };

    struct AudioAsset : public Asset {
        uint32_t sampleRate;
        uint32_t channels;
        std::vector<float> samples{};

        virtual AssetType type() const override {
            return AssetType::Audio;
        }
    };

}
