#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"

namespace muon {

    class MuonEditor final : public Application {
    public:
        MuonEditor(const Spec &spec) : Application(spec) {

        }
    };

    auto CreateApplication(ApplicationCommandLineArgs args) -> Application * {
        Application::Spec spec{};
        spec.name = "Muon Editor";
        spec.cliArgs = args;

        return new MuonEditor(spec);
    }

}
