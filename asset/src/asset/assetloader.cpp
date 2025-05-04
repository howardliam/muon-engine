#include "muon/asset/assetloader.hpp"

#include "muon/asset/image/jpeg.hpp"
#include "muon/asset/image/png.hpp"
#include <memory>
#include <print>

namespace muon::asset {

    AssetLoader::AssetLoader() {

        registerHandler(std::make_unique<PngHandler>());
        registerHandler(std::make_unique<JpegHandler>());

    }

    AssetLoader::~AssetLoader() {}

    void AssetLoader::registerHandler(std::unique_ptr<IAssetHandler> handler) {
        handlers.push_back(std::move(handler));
    }

    std::shared_ptr<Asset> AssetLoader::loadAsset(const std::filesystem::path &path) const {
        auto extension = getExtension(path);
        if (!extension) {
            return nullptr;
        }

        for (auto &handler : handlers) {
            if (handler->supports(*extension)) {
                return handler->load(path);
            }
        }

        return nullptr;
    }


    std::optional<std::string> AssetLoader::getExtension(const std::filesystem::path &path) const {
        if (!path.has_extension()) {
            return {};
        }
        return path.extension().string().erase(0, 1);
    }
}
