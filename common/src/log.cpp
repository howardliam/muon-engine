#include "muon/common/log.hpp"

namespace muon::common::log {

    std::optional<int32_t> deleteOldLogs(std::filesystem::path directory, size_t maxHistory, std::chrono::day maxAge) {
        return {};
    }

}
