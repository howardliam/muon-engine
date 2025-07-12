#pragma once

namespace muon {

class NoCopy {
protected:
    NoCopy() = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy &other) = delete;
    NoCopy &operator=(const NoCopy &other) = delete;
};

} // namespace muon
