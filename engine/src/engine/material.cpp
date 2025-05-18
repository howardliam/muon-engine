#include "muon/engine/material.hpp"

namespace muon::engine {

    uint16_t operator&(uint16_t lhs, Material::TextureMaskBits rhs) {
        return lhs & static_cast<uint16_t>(rhs);
    }

    uint16_t operator|=(uint16_t lhs, Material::TextureMaskBits rhs) {
        lhs |= static_cast<uint16_t>(rhs);
        return lhs;
    }

    bool Material::has(TextureMaskBits texture) {
        return textureMask & texture;
    }

    Material::Builder &Material::Builder::withAlbedo(uint16_t index) {
        material.albedo = index;
        material.textureMask |= TextureMaskBits::Albedo;
        return *this;
    }

    Material::Builder &Material::Builder::withNormal(uint16_t index) {
        material.normal = index;
        material.textureMask |= TextureMaskBits::Normal;
        return *this;
    }

    Material::Builder &Material::Builder::withRoughness(uint16_t index) {
        material.roughness = index;
        material.textureMask |= TextureMaskBits::Roughness;
        return *this;
    }

    Material::Builder &Material::Builder::withMetalness(uint16_t index) {
        material.metalness = index;
        material.textureMask |= TextureMaskBits::Metalness;
        return *this;
    }

    Material::Builder &Material::Builder::withHeight(uint16_t index) {
        material.height = index;
        material.textureMask |= TextureMaskBits::Height;
        return *this;
    }

    Material::Builder &Material::Builder::withEmissive(uint16_t index) {
        material.emissive = index;
        material.textureMask |= TextureMaskBits::Emissive;
        return *this;
    }

    Material Material::Builder::build() const {
        return material;
    }

    std::shared_ptr<Material> Material::Builder::buildSharedPtr() const {
        return std::make_shared<Material>(material);
    }

}
