#pragma once

namespace muon {

struct no_move {
    no_move() = default;

    no_move(no_move &&) = delete;
    auto operator=(no_move &&) -> no_move & = delete;
};

} // namespace muon
