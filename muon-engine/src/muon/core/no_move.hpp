#pragma once

namespace muon {

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&other) = delete;
    auto operator=(NoMove &&other) -> NoMove & = delete;
};

struct no_move {
    no_move() = default;

    no_move(no_move &&) = delete;
    auto operator=(no_move &&) -> no_move & = delete;
};

} // namespace muon
