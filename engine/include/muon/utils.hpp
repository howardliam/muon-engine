#pragma once

namespace muon {

    class NoCopy {
    protected:
        NoCopy(const NoCopy &other) = delete;
        NoCopy &operator=(const NoCopy &other) = delete;
    };

    class NoMove {
    protected:
        NoMove(NoMove &&other) = delete;
        NoMove &&operator=(NoMove &&other) = delete;
    };

}
