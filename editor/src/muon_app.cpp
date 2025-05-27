#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"

namespace muon {

    class MuonEditor final : public Application {
    public:
        MuonEditor(const Specification &spec) : Application(spec) {

        }
    };


    Application *createApplication(Application::CommandLineArgs args) {
        Application::Specification spec{};
        spec.name = "Muon Editor";
        spec.cliArgs = args;

        return new MuonEditor(spec);
    }

}
