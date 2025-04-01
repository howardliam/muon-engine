#include <filesystem>

#include <muon/common/log.hpp>
#include <muon/common/fs.hpp>
#include <muon/common/compress.hpp>

// namespace log = muon::common::log;
namespace fs = muon::common::fs;

int main() {
    std::filesystem::path testFile("test.txt");
    fs::compressFile(testFile);
}
