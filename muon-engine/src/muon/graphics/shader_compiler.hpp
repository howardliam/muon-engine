#pragma once

#include "SQLiteCpp/Database.h"
#include "muon/crypto/crypto.hpp"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>

namespace muon::graphics {

struct ShaderCompilationRequest {
    std::filesystem::path path;
};

class ShaderCompiler {
public:
    struct Spec {
        std::filesystem::path hashStorePath;
    };

public:
    ShaderCompiler(const Spec &spec);
    ~ShaderCompiler();

    auto submitWork(ShaderCompilationRequest request) -> void;

private:
    auto compile(const ShaderCompilationRequest &request) -> void;

private:
    SQLite::Database m_hashStore;
    crypto::Crypto m_crypto;

    std::queue<ShaderCompilationRequest> m_workQueue{};
    std::mutex m_workMutex;
    std::condition_variable m_conVar;

    std::atomic<bool> m_terminate{false};
    std::thread m_worker;
};

} // namespace muon::graphics
