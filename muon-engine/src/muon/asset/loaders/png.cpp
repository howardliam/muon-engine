#include "muon/asset/loaders/png.hpp"

#include "muon/asset/asset_manager.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/texture.hpp"
#include "spng.h"
#include "vulkan/vulkan_structs.hpp"

namespace muon::asset {

auto PngLoader::getFileTypes() const -> std::set<std::string_view> { return {".png"}; }

auto PngLoader::fromMemory(const std::vector<uint8_t> &data) -> void {
    spng_ctx *ctx = spng_ctx_new(0);

    spng_set_png_buffer(ctx, data.data(), data.size());

    spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);

    spng_format format;
    uint8_t channels = 0;
    switch (ihdr.color_type) {
        case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
            format = SPNG_FMT_RGBA8;
            channels = 4;
            break;

        case SPNG_COLOR_TYPE_TRUECOLOR:
            format = SPNG_FMT_RGB8;
            channels = 3;
            break;

        case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
            format = SPNG_FMT_GA8;
            channels = 2;
            break;

        case SPNG_COLOR_TYPE_GRAYSCALE:
            format = SPNG_FMT_G8;
            channels = 1;
            break;

        default:
            break;
    }

    size_t imageSize;
    spng_decoded_image_size(ctx, format, &imageSize);

    std::vector<uint8_t> decodedData(imageSize);
    spng_decode_image(ctx, decodedData.data(), decodedData.size(), format, 0);

    spng_ctx_free(ctx);

    graphics::Texture::Spec spec{m_manager->getContext(), m_manager->getCommandBuffer(), std::move(decodedData)};
    spec.uploadBuffers = m_manager->getUploadBuffers();
    spec.extent = vk::Extent2D{ihdr.width, ihdr.height};
    spec.pixelSize = channels;
    spec.format = [](const uint8_t &channels) -> vk::Format {
        switch (channels) {
            case 4:
                return vk::Format::eR8G8B8A8Srgb;

            case 3:
                return vk::Format::eR8G8B8Srgb;

            case 2:
                return vk::Format::eR8G8Srgb;

            case 1:
                return vk::Format::eR8Srgb;

            default:
                return vk::Format::eUndefined;
        }
    }(channels);

    m_manager->getTextures().emplace_back(spec);
}

auto PngLoader::fromFile(const std::filesystem::path &path) -> void { core::debug("PngLoader::FromFile successfully called"); }

} // namespace muon::asset
