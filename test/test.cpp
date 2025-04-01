#include <print>
#include <filesystem>

#include <muon/common/log.hpp>
#include <muon/common/fs.hpp>

using namespace std::chrono_literals;

namespace log = muon::common::log;
namespace fs = muon::common::fs;

int main() {

    std::filesystem::path path;

    log::deleteOldLogs(path, 5, 5d);

    if (fs::compressFile(path)) {
        std::println("Hello!");
    }
}
