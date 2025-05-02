#pragma once

#include "muon/asset/model/scene/sampler.hpp"
#include <optional>
#include <cstdint>
#include <string>

namespace muon::asset {

    struct Texture {
        std::optional<Sampler> sampler{};
        std::optional<int32_t> source{};
        std::optional<std::string> name{};
    };

    struct TextureInfo {
        int32_t index{0};
        std::optional<int32_t> texCoord{};
    };

    struct NormalTextureInfo {
        int32_t index{0};
        std::optional<int32_t> texCoord{};
        std::optional<float> scale{};
    };

    struct OcclusionTextureInfo {
        int32_t index{0};
        std::optional<int32_t> texCoord{};
        std::optional<float> strength{};
    };
}
