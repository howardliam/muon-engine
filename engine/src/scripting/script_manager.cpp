#include "muon/scripting/script_manager.hpp"

namespace muon {

    ScriptManager::ScriptManager() {
        m_lua.open_libraries(sol::lib::base, sol::lib::package);
        m_script = m_lua.load_file("assets/scripts/test.lua");
    }

    ScriptManager::~ScriptManager() {
    }

    void ScriptManager::run() {
        m_script();
    }

}
