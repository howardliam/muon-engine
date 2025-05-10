#pragma once

namespace muon {

    class NoCopy {
    protected:
        NoCopy() = default;
        ~NoCopy() = default;

        NoCopy(const NoCopy &other) = delete;
        NoCopy &operator=(const NoCopy &other) = delete;
    };

    class NoMove {
    protected:
        NoMove() = default;
        ~NoMove() = default;

        NoMove(NoMove &&other) = delete;
        NoMove &&operator=(NoMove &&other) = delete;
    };

}
