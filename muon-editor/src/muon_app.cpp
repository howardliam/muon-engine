#include "muon/core/application.hpp"
#include "muon/core/entry_point.hpp"

#include <filesystem>

namespace muon {

class MuonEditor final : public Application {
public:
    MuonEditor(const Spec &spec) : Application(spec) {}
};

auto createApplication(const std::vector<const char *> &args) -> Application * {
    Application::Spec spec{};
    spec.name = "Muon Editor";
    spec.workingDirectory = std::filesystem::current_path();
    spec.args = args;

    return new MuonEditor(spec);
}

} // namespace muon
