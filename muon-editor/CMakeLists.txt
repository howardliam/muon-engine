add_executable(editor
    src/muon_app.cpp
)

target_link_libraries(editor PRIVATE muon::engine)
