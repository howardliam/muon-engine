#include "muon/asset/loaders/png.hpp"

#include "muon/asset/manager.hpp"
#include "muon/core/log.hpp"

#include <spng.h>

namespace muon::asset {

auto PngLoader::GetFileTypes() const -> std::set<std::string_view> { return {".png"}; }

auto PngLoader::FromMemory(const std::vector<uint8_t> &data) -> void {
    MU_CORE_DEBUG("PngLoader::FromMemory successfully called");
    // spng_ctx *ctx = spng_ctx_new(0);

    // spng_set_png_buffer(ctx, data.data(), data.size());

    // spng_ihdr ihdr;
    // spng_get_ihdr(ctx, &ihdr);

    // size_t imageSize;

    // spng_ctx_free(ctx);
}

auto PngLoader::FromFile(const std::filesystem::path &path) -> void { MU_CORE_DEBUG("PngLoader::FromFile successfully called"); }

} // namespace muon::asset
