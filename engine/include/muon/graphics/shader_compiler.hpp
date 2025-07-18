#pragma once

#include <SQLiteCpp/Database.h>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <glslang/Include/ResourceLimits.h>
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

    auto SubmitWork(ShaderCompilationRequest request) -> void;

private:
    auto Compile(const ShaderCompilationRequest &request) -> void;

private:
    TBuiltInResource m_resource;

    SQLite::Database m_hashStore;

    std::queue<ShaderCompilationRequest> m_workQueue{};
    std::mutex m_workMutex;
    std::condition_variable m_conVar;

    std::atomic<bool> m_terminate{false};
    std::thread m_worker;
};

} // namespace muon::graphics
