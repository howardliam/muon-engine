#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace muon::asset {

    enum class Filter : int32_t {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear,
    };

    enum class WrappingMode : int32_t {
        ClampToEdge,
        MirroredRepeat,
        Repeat,
    };

    struct Sampler {
        std::optional<Filter> magFilter;
        std::optional<Filter> minFilter;
        std::optional<WrappingMode> wrapS;
        std::optional<WrappingMode> wrapT;
        std::optional<std::string> name;
    };

}
