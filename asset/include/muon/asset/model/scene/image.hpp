#pragma once

#include "muon/asset/image.hpp"
#include <optional>
#include <string>

namespace muon::asset::scene {

    struct Image {
        asset::Image image;
        std::optional<std::string> name;
    };

}
