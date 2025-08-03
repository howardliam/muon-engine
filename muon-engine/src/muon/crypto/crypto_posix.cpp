#include "muon/crypto/crypto.hpp"

#include "muon/core/expect.hpp"
#include "muon/utils/library.hpp"

#include <openssl/evp.h>

namespace muon::crypto {

void *g_libraryHandle;

using InitCryptoFn = int32_t (*)(uint64_t, void *);
InitCryptoFn OPENSSL_init_crypto;

using GetDigestByNameFn = void * (*)(const char *);
GetDigestByNameFn EVP_get_digestbyname;

using MdCtxNewFn = void * (*)();
MdCtxNewFn EVP_MD_CTX_new;

using MdCtxFreeFn = void (*)(void *);
MdCtxFreeFn EVP_MD_CTX_free;

using DigestInitFn = int32_t (*)(void *, const void *, void *);
DigestInitFn EVP_DigestInit_ex;

using DigestUpdateFn = int32_t (*)(void *, const void *, size_t);
DigestUpdateFn EVP_DigestUpdate;

using DigestFinalFn = int32_t (*)(void *, uint8_t *, uint32_t *);
DigestFinalFn EVP_DigestFinal_ex;

void loadFunctionPointers() {
    auto openResult = utils::openDynamicLibrary("libcrypto.so");
    core::expect(openResult, "failed to open library: libcrypto.so");
    g_libraryHandle = *openResult;

    auto loadResult = utils::loadSymbol(g_libraryHandle, "OPENSSL_init_crypto");
    core::expect(loadResult, "failed to load symbol: OPENSSL_init_crypto");
    OPENSSL_init_crypto = reinterpret_cast<InitCryptoFn>(*loadResult);

    OPENSSL_init_crypto(0x00000008, nullptr);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_get_digestbyname");
    core::expect(loadResult, "failed to load symbol: EVP_get_digestbyname");
    EVP_get_digestbyname = reinterpret_cast<GetDigestByNameFn>(*loadResult);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_MD_CTX_new");
    core::expect(loadResult, "failed to load symbol: EVP_MD_CTX_new");
    EVP_MD_CTX_new = reinterpret_cast<MdCtxNewFn>(*loadResult);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_MD_CTX_free");
    core::expect(loadResult, "failed to load symbol: EVP_MD_CTX_free");
    EVP_MD_CTX_free = reinterpret_cast<MdCtxFreeFn>(*loadResult);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestInit_ex");
    core::expect(loadResult, "failed to load symbol: EVP_DigestInit_ex");
    EVP_DigestInit_ex = reinterpret_cast<DigestInitFn>(*loadResult);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestUpdate");
    core::expect(loadResult, "failed to load symbol: EVP_DigestUpdate");
    EVP_DigestUpdate = reinterpret_cast<DigestUpdateFn>(*loadResult);

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestFinal_ex");
    core::expect(loadResult, "failed to load symbol: EVP_DigestFinal_ex");
    EVP_DigestFinal_ex = reinterpret_cast<DigestFinalFn>(*loadResult);
}

void init() {
    std::call_once(s_loadFlag, loadFunctionPointers);
}

void cleanup() {
    auto closeResult = utils::closeDynamicLibrary(g_libraryHandle);
    core::expect(closeResult, "failed to close library: libcrypto.so");
}

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError> {
    const void *md = EVP_get_digestbyname("sha256");
    void *ctx = EVP_MD_CTX_new();

    int32_t result = EVP_DigestInit_ex(ctx, md, nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::InitializationFailure);
    }

    result = EVP_DigestUpdate(ctx, data, size);
    if (result == 0) {
        return std::unexpected(CryptoError::ProcessingFailure);
    }

    std::array<uint8_t, k_hashSize> output;
    result = EVP_DigestFinal_ex(ctx, output.data(), nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::FinalizationFailure);
    }

    EVP_MD_CTX_free(ctx);

    return output;
}

auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, k_hashSize>, CryptoError> {
    const void *md = EVP_get_digestbyname("sha256");
    void *ctx = EVP_MD_CTX_new();

    int32_t result = EVP_DigestInit_ex(ctx, md, nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::InitializationFailure);
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        result = EVP_DigestUpdate(ctx, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result == 0) {
            return std::unexpected(CryptoError::ProcessingFailure);
        }
    }

    std::array<uint8_t, k_hashSize> output;
    result = EVP_DigestFinal_ex(ctx, output.data(), nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::FinalizationFailure);
    }

    EVP_MD_CTX_free(ctx);

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    return output;
}

}
