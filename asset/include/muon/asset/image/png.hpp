#pragma once

#include "muon/asset/assetloader.hpp"

namespace muon::asset {

    struct PngHandler : public IAssetHandler {

        virtual bool supports(const std::string &extension) const override;

        virtual std::shared_ptr<Asset> load(const std::filesystem::path &path) override;

    };

}
