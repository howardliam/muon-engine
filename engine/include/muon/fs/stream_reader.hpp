#pragma once

#include <cstdint>

namespace muon::fs {

    class StreamReader {
    public:
        virtual ~StreamReader() = default;

        [[nodiscard]] virtual auto GetStreamPosition() const -> uint64_t = 0;
        virtual auto SetStreamPosition(uint64_t position) -> void = 0;
    };

}
