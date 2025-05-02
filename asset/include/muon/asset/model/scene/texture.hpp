#pragma once

#include "muon/asset/model/scene/image.hpp"
#include "muon/asset/model/scene/sampler.hpp"
#include <optional>
#include <cstdint>
#include <string>

namespace muon::asset::scene {

    struct Texture {
        std::shared_ptr<Sampler> sampler{nullptr};
        std::shared_ptr<Image> image{nullptr};
        std::optional<std::string> name{};
    };

    struct TextureInfo {
        std::shared_ptr<Texture> texture{nullptr};
        std::optional<int32_t> texCoord{};
    };

    struct NormalTextureInfo {
        std::shared_ptr<Texture> texture{nullptr};
        std::optional<int32_t> texCoord{};
        std::optional<float> scale{};
    };

    struct OcclusionTextureInfo {
        std::shared_ptr<Texture> texture{nullptr};
        std::optional<int32_t> texCoord{};
        std::optional<float> strength{};
    };

}
