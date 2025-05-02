#pragma once

#include <optional>
#include <string>

namespace muon::asset {

    struct Material {
        std::optional<std::string> name{};
        // pbr metal roughness
        // normal tex
        // occlusion tex
        // emissive tex
        // emissive factor
        // alpha mode
        // alpha cutoff
        // double sided
    };

}
