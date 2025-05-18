add_executable(test
    src/main.cpp
    src/logger.hpp
    src/logger.cpp
)

target_link_libraries(test PRIVATE spdlog::spdlog_header_only muon-engine muon-common muon-asset)
