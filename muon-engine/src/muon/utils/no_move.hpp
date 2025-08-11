#pragma once

namespace muon::utils {

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&) = delete;
    auto operator=(NoMove &&) -> NoMove & = delete;
};

} // namespace muon::utils
