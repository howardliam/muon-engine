#pragma once

#include <cstdint>
#include <vector>
namespace muon::asset {

    struct Audio {
        uint32_t sampleRate;
        uint32_t channels;
        std::vector<float> samples{};
    };

}
