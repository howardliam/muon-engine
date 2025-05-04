#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace muon::asset {

    enum class AssetType {
        Image,
        Model,
        Audio,
        Font,
        Unknown
    };

    struct Asset {
        virtual ~Asset() = default;
        virtual AssetType type() const = 0;

        template<typename T>
        T *get(AssetType expected) {
            return (type() == expected) ? static_cast<T *>(this) : nullptr;
        }
    };

    struct IAssetHandler {
        virtual ~IAssetHandler() = default;

        virtual bool supports(const std::string &extension) const = 0;

        virtual std::shared_ptr<Asset> load(const std::filesystem::path &path) = 0;
    };

    class AssetLoader {
    public:
        AssetLoader();
        ~AssetLoader();

        void registerHandler(std::unique_ptr<IAssetHandler> handler);

        std::shared_ptr<Asset> loadAsset(const std::filesystem::path &path) const;
        // std::shared_ptr<Asset> loadAsset(const std::vector<uint8_t> &data) const;

    private:
        std::vector<std::unique_ptr<IAssetHandler>> handlers{};

        std::optional<std::string> getExtension(const std::filesystem::path &path) const;
        // std::optional<std::string> determineExtension(const std::vector<uint8_t> &data) const;

    };


}
