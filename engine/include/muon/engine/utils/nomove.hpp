#pragma once

namespace mu {

    class NoMove {
    protected:
        NoMove() = default;
        ~NoMove() = default;

        NoMove(NoMove &&other) = delete;
        NoMove &operator=(NoMove &&other) = delete;
    };

}
