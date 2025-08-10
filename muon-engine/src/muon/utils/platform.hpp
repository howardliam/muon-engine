#pragma once

#include <expected>
#include <filesystem>
#include <string_view>

namespace muon::utils {

enum class library_error {
    library_open_failure,
    symbol_load_failure,
    library_close_failure,
};

using library_handle = void *;
using symbol_handle = void *;

auto open_library(const std::filesystem::path &path) -> std::expected<library_handle, library_error>;
auto load_symbol(library_handle handle, std::string_view name) -> std::expected<symbol_handle, library_error>;
auto close_library(library_handle handle) -> std::expected<void, library_error>;

enum class signal {
    debug_trap,
};

void invoke_signal(signal signal);

auto has_elevated_privileges() -> bool;

} // namespace muon
