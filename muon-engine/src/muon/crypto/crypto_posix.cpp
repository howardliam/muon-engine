#include "muon/crypto/crypto.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/utils/library.hpp"

#include <openssl/evp.h>

namespace muon::crypto {

void *g_libraryHandle;

using InitCryptoFn = int32_t (*)(uint64_t, void *);
InitCryptoFn initCrypto;

using Sha256Fn = uint8_t * (*)(const uint8_t *, size_t, uint8_t *);
Sha256Fn sha256;

using GetDigestByNameFn = void * (*)(const char *);
GetDigestByNameFn getDigestByName;

using MdCtxNewFn = void * (*)();
MdCtxNewFn mdCtxNew;

using MdCtxFreeFn = void (*)(void *);
MdCtxFreeFn mdCtxFree;

using DigestInitFn = int32_t (*)(void *, const void *, void *);
DigestInitFn digestInit;

using DigestUpdateFn = int32_t (*)(void *, const void *, size_t);
DigestUpdateFn digestUpdate;

using DigestFinalFn = int32_t (*)(void *, uint8_t *, uint32_t *);
DigestFinalFn digestFinal;

void loadFunctionPointers() {
    auto loadResult = utils::openDynamicLibrary("libcrypto.so");
    core::expect(loadResult, "failed to open library: libcrypto.so");
    g_libraryHandle = *loadResult;
    core::trace("opened library: libcrypto.so");

    loadResult = utils::loadSymbol(g_libraryHandle, "OPENSSL_init_crypto");
    core::expect(loadResult, "failed to load symbol: OPENSSL_init_crypto");
    initCrypto = reinterpret_cast<InitCryptoFn>(*loadResult);
    core::trace("loaded symbol: OPENSSL_init_crypto");

    initCrypto(0x00000008, nullptr);

    loadResult = utils::loadSymbol(g_libraryHandle, "SHA256");
    core::expect(loadResult, "failed to load symbol: SHA256");
    sha256 = reinterpret_cast<Sha256Fn>(*loadResult);
    core::trace("loaded symbol: SHA256");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_get_digestbyname");
    core::expect(loadResult, "failed to load symbol: EVP_get_digestbyname");
    getDigestByName = reinterpret_cast<GetDigestByNameFn>(*loadResult);
    core::trace("loaded symbol: EVP_get_digestbyname");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_MD_CTX_new");
    core::expect(loadResult, "failed to load symbol: EVP_MD_CTX_new");
    mdCtxNew = reinterpret_cast<MdCtxNewFn>(*loadResult);
    core::trace("loaded symbol: EVP_MD_CTX_new");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_MD_CTX_free");
    core::expect(loadResult, "failed to load symbol: EVP_MD_CTX_free");
    mdCtxFree = reinterpret_cast<MdCtxFreeFn>(*loadResult);
    core::trace("loaded symbol: EVP_MD_CTX_free");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestInit_ex");
    core::expect(loadResult, "failed to load symbol: EVP_DigestInit_ex");
    digestInit = reinterpret_cast<DigestInitFn>(*loadResult);
    core::trace("loaded symbol: EVP_DigestInit_ex");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestUpdate");
    core::expect(loadResult, "failed to load symbol: EVP_DigestUpdate");
    digestUpdate = reinterpret_cast<DigestUpdateFn>(*loadResult);
    core::trace("loaded symbol: EVP_DigestUpdate");

    loadResult = utils::loadSymbol(g_libraryHandle, "EVP_DigestFinal_ex");
    core::expect(loadResult, "failed to load symbol: EVP_DigestFinal_ex");
    digestFinal = reinterpret_cast<DigestFinalFn>(*loadResult);
    core::trace("loaded symbol: EVP_DigestFinal_ex");
}

Crypto::Crypto() {
    std::call_once(s_loadFlag, loadFunctionPointers);
}

Crypto::~Crypto() {}

auto Crypto::hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, 32>, CryptoError> {
    std::array<uint8_t, 32> output;
    sha256(data, size, output.data());

    return output;
}

auto Crypto::hash(std::ifstream &file) -> std::expected<std::array<uint8_t, 32>, CryptoError> {
    const void *md = getDigestByName("sha256");
    void *ctx = mdCtxNew();

    int32_t result = digestInit(ctx, md, nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::InitializationFailure);
    }

    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        result = digestUpdate(ctx, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result == 0) {
            return std::unexpected(CryptoError::ProcessingFailure);
        }
    }

    std::array<uint8_t, 32> output;
    result = digestFinal(ctx, output.data(), nullptr);
    if (result == 0) {
        return std::unexpected(CryptoError::FinalizationFailure);
    }

    mdCtxFree(ctx);

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    return output;
}

}
