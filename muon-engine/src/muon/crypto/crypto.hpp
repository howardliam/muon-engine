#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include <mutex>

namespace {
    static inline std::once_flag s_initFlag;
    static inline std::once_flag s_cleanupFlag;

    constexpr const char *k_hashDigest = "sha256";
}

namespace muon::crypto {

constexpr size_t k_hashSize = 32;

enum class CryptoError {
    InitializationFailure,
    ProcessingFailure,
    FinalizationFailure,
};

void init();
void cleanup();

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError>;
auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError>;

}
