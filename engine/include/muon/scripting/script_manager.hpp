#pragma once

#include <sol/sol.hpp>

namespace muon {

    class ScriptManager {
    public:
        ScriptManager();
        ~ScriptManager();

        void Run();

    private:
        sol::state m_lua;
        sol::load_result m_script;
    };

}
