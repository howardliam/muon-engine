#pragma once

namespace muon {

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&other) = delete;
    auto operator=(NoMove &&other) -> NoMove & = delete;
};

} // namespace muon
