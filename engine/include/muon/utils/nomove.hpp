#pragma once

namespace muon {

    class NoMove {
    protected:
        NoMove() = default;
        ~NoMove() = default;

        NoMove(NoMove &&other) = delete;
        NoMove &&operator=(NoMove &&other) = delete;
    };

}
