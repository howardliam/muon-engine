#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"

namespace muon::fg {

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
