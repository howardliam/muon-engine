#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>

namespace muon::engine::fg {

    class Blackboard {
    public:
        template<typename T, typename ...Args>
        T &add(Args &&...args);

        template<typename T>
        const T &get() const;

    private:
        std::unordered_map<std::type_index, std::any> storage;
    };

}

#include "muon/engine/renderer/frame_graph/blackboard.inl"
