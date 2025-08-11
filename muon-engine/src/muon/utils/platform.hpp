#pragma once

#include <expected>
#include <filesystem>
#include <string_view>

namespace muon::utils {

enum class LibraryError {
    LibraryOpenFailure,
    SymbolLoadFailure,
    LibraryCloseFailure,
};

using LibraryHandle = void *;
using SymbolHandle = void *;

auto open_library(const std::filesystem::path &path) -> std::expected<LibraryHandle, LibraryError>;
auto load_symbol(LibraryHandle handle, std::string_view name) -> std::expected<SymbolHandle, LibraryError>;
auto close_library(LibraryHandle handle) -> std::expected<void, LibraryError>;

enum class Signal {
    DebugTrap,
};

void invoke_signal(Signal signal);

auto has_elevated_privileges() -> bool;

} // namespace muon
