#pragma once

namespace muon {

struct no_copy {
    no_copy() = default;

    no_copy(const no_copy &) = delete;
    auto operator=(const no_copy &) -> no_copy & = delete;
};

} // namespace muon
