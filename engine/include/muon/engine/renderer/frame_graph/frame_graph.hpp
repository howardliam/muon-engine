#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"

namespace muon::engine::fg {

    class FrameGraph : NoCopy, NoMove {
    public:
        class Builder;

        FrameGraph();
        ~FrameGraph();

        void compile();

        void execute();

    private:

    };

    class FrameGraph::Builder : NoCopy, NoMove {
    public:



    private:

        friend class FrameGraph;
    };

}
