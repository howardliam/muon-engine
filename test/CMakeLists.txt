add_executable(test
    test.cpp
)

target_link_libraries(test PRIVATE spdlog::spdlog_header_only muon-engine muon-common muon-asset)
