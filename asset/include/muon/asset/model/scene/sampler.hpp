#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace muon::asset {

    struct Sampler {
        std::optional<int32_t> magFilter{};
        std::optional<int32_t> minFilter{};
        std::optional<int32_t> wrapS{};
        std::optional<int32_t> wrapT{};
        std::optional<std::string> name{};
    };

}
