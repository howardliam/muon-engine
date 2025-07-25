#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"

#include <filesystem>

namespace muon {

class MuonEditor final : public Application {
public:
    MuonEditor(const Spec &spec) : Application(spec) {}
};

auto createApplication(ApplicationCommandLineArgs args) -> Application * {
    Application::Spec spec{};
    spec.name = "Muon Editor";
    spec.cliArgs = args;
    spec.workingDirectory = std::filesystem::current_path();

    return new MuonEditor(spec);
}

} // namespace muon
