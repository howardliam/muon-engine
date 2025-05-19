add_executable(test
    src/g_buffer.hpp
    src/g_buffer.cpp
    src/logger.hpp
    src/logger.cpp
    src/main.cpp
)

target_link_libraries(test PRIVATE spdlog::spdlog_header_only muon-engine muon-common muon-asset)
