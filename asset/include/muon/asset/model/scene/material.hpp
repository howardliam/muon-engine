#pragma once

#include "muon/asset/model/scene/texture.hpp"
#include <array>
#include <optional>
#include <string>

namespace muon::asset {

    struct PbrMetallicRoughness {
        std::optional<std::array<float, 4>> baseColorFactor{};
        std::optional<TextureInfo> baseColorTexture{};
        std::optional<float> metallicFactor{};
        std::optional<float> roughnessFactor{};
        std::optional<TextureInfo> metallicRoughnessTexture{};
    };

    enum class AlphaMode {
        Opaque,
        Mask,
        Blend,
    };

    struct Material {
        std::optional<std::string> name{};
        std::optional<PbrMetallicRoughness> pbrMetallicRoughness{};
        std::optional<NormalTextureInfo> normalTexture{};
        std::optional<OcclusionTextureInfo> occlusionTexture{};
        std::optional<TextureInfo> emissiveTexture{};
        std::optional<std::array<float, 3>> emissiveFactor{};
        std::optional<AlphaMode> alphaMode{};
        std::optional<float> alphaCutoff{};
        std::optional<bool> doubleSided{};
    };

}
