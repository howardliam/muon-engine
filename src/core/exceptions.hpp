#pragma once

#include <stdexcept>

namespace muon::except {

    class CompressionFailedException : public std::runtime_error {
    public:
        explicit CompressionFailedException() : std::runtime_error("Compression failed") {}
        explicit CompressionFailedException(std::string_view msg) : std::runtime_error(msg.data()) {}
    };
}
