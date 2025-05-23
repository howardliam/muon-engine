#include "muon/common/log.hpp"

namespace mu::common::log {

    std::optional<int32_t> deleteOldLogs(std::filesystem::path directory, size_t maxHistory, std::chrono::day maxAge) {
        return {};
    }

}
