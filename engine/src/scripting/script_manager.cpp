#include "muon/scripting/script_manager.hpp"

#include "muon/core/log.hpp"

namespace muon {

    ScriptManager::ScriptManager() {
        m_lua.open_libraries(sol::lib::base, sol::lib::package);

        auto test = []() {
            MU_INFO("hello from Lua");
        };

        m_lua.set_function("hello", test);

        m_script = m_lua.load_file("assets/scripts/test.lua");
    }

    ScriptManager::~ScriptManager() {
    }

    void ScriptManager::run() {
        m_script();
    }

}
