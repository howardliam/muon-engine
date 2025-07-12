#include "muon/asset/loaders/png.hpp"

#include "muon/asset/manager.hpp"

#include <spng.h>

namespace muon::asset {

auto PngLoader::GetFileType() -> std::string_view { return ".png"; }

auto PngLoader::FromMemory(const std::vector<uint8_t> &data) -> void {

    spng_ctx *ctx = spng_ctx_new(0);

    spng_set_png_buffer(ctx, data.data(), data.size());

    spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);

    size_t imageSize;

    spng_ctx_free(ctx);
}

auto PngLoader::FromFile(const std::filesystem::path &path) -> void {}

} // namespace muon::asset
