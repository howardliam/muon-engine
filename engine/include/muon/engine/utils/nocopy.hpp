#pragma once

namespace mu {

    class NoCopy {
    protected:
        NoCopy() = default;
        ~NoCopy() = default;

        NoCopy(const NoCopy &other) = delete;
        NoCopy &operator=(const NoCopy &other) = delete;
    };

}
