#pragma once

namespace muon {

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &other) = delete;
    auto operator=(const NoCopy &other) -> NoCopy & = delete;
};

struct no_copy {
    no_copy() = default;

    no_copy(const no_copy &) = delete;
    auto operator=(const no_copy &) -> no_copy & = delete;
};

} // namespace muon
