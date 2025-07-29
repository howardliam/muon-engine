#pragma once

namespace muon {

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &other) = delete;
    auto operator=(const NoCopy &other) -> NoCopy & = delete;
};

} // namespace muon
