#pragma once

namespace muon::utils {

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    auto operator=(const NoCopy &) -> NoCopy & = delete;
};

} // namespace muon::utils
