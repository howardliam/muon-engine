#pragma once

#include <filesystem>
#include <chrono>
#include <optional>

namespace mu::common::log {

    /**
     * @brief   Deletes old logs in the target directory.
     *
     * First deletes all logs over max history, if any: 20 logs but max 15, five will be deleted.
     * Next deletes all older than the max age.
     *
     * @param   directory   directory to delete from.
     * @param   maxHistory  max number of files to retain.
     * @param   maxAge      max age in days of files to retain.
     *
     * @return  option containing the number of files deleted. If anything went wrong, nothing will be returned.
    */
    std::optional<int32_t> deleteOldLogs(std::filesystem::path directory, size_t maxHistory, std::chrono::day maxAge);

}
