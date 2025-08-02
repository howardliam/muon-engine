#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include <mutex>

namespace muon::crypto {

constexpr size_t k_hashSize = 32;

enum class CryptoError {
    InitializationFailure,
    ProcessingFailure,
    FinalizationFailure,
};

class Crypto {
public:
    Crypto();
    ~Crypto();

    auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError>;
    auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError>;

private:
    static inline std::once_flag s_loadFlag;
};


}
