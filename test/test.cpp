#include <print>
#include <filesystem>

#include <muon/common/log.hpp>
#include <muon/common/fs.hpp>
#include <muon/common/compress.hpp>

using namespace std::chrono_literals;

namespace log = muon::common::log;
namespace fs = muon::common::fs;
namespace compress = muon::common::compress;

int main() {

    std::filesystem::path path;

    log::deleteOldLogs(path, 5, 5d);

    if (fs::compressFile(path)) {
        std::println("Hello!");
    }

    std::vector<char> buffer = {'H', 'E', 'L', 'L', 'O'};
    auto result = compress::compressBuffer(buffer);
    if (result.has_value()) {
        std::println("Got result: {}", result.value().data());
    }
}
