#include "muon/asset/assetloader.hpp"
#include "muon/asset/image.hpp"
#include <print>

using namespace muon;

int main() {

    asset::AssetLoader loader;

    std::shared_ptr asset = loader.loadAsset("./muon-logo.jpg");
    auto image = asset->get<asset::ImageAsset>(asset::AssetType::Image);
    if (image == nullptr) {
        return 1;
    }
    std::println("Loaded an image");
    std::println("Dimensions: {}x{}", image->width, image->height);
    std::println("Channels: {}", image->channels);
}
