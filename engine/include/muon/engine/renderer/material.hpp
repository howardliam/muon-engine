#pragma once

#include <cstdint>
#include <memory>

namespace muon {

    struct Material {
        class Builder;
        enum class TextureMaskBits : uint16_t;

        uint16_t albedo;
        uint16_t normal;
        uint16_t roughness;
        uint16_t metalness;
        uint16_t height;
        uint16_t emissive;

        uint16_t textureMask;

        bool has(TextureMaskBits texture);
    };

    class Material::Builder {
    public:
        Builder &withAlbedo(uint16_t index);
        Builder &withNormal(uint16_t index);
        Builder &withRoughness(uint16_t index);
        Builder &withMetalness(uint16_t index);
        Builder &withHeight(uint16_t index);
        Builder &withEmissive(uint16_t index);

        Material build() const;
        std::shared_ptr<Material> buildSharedPtr() const;

    private:
        Material material{};
    };

    enum class Material::TextureMaskBits : uint16_t {
        Albedo = 1 << 0,
        Normal = 1 << 1,
        Roughness = 1 << 2,
        Metalness = 1 << 3,
        Height = 1 << 4,
        Emissive = 1 << 5,
    };

}
