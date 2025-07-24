#include "muon/core/project.hpp"

#include "muon/core/errors.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

namespace muon {

Project::Project(const Spec &spec) : m_name(spec.name), m_path(spec.path) {}

Project::~Project() {
    auto success = Save();
    core::expect(success, "failed to save project file to disk");
}

auto Project::Create(const Spec &spec) -> std::expected<std::shared_ptr<Project>, ProjectError> {
    s_activeProject = std::make_shared<Project>(spec);

    auto result = s_activeProject->ConfigureProjectStructure();
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    result = s_activeProject->WriteProjectFile();
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    core::debug("created new project");

    return s_activeProject;
}

auto Project::Load(const std::filesystem::path &projectPath) -> std::expected<std::shared_ptr<Project>, ProjectError> {
    if (!std::filesystem::exists(projectPath)) {
        core::trace("no project exists, attempting to create");
        Spec spec{};
        spec.path = projectPath;
        return Create(spec);
    }

    auto projectConfigPath = projectPath / "project.toml";
    if (!std::filesystem::exists(projectConfigPath)) {
        return std::unexpected(ProjectError::ProjectFileDoesNotExist);
    }

    auto config = toml::parse_file(projectConfigPath.c_str());

    auto projectName = config["name"].value<std::string_view>();
    if (!projectName.has_value()) {
        return std::unexpected(ProjectError::MalformedProjectFile);
    }

    Spec spec{};
    spec.path = projectPath;
    spec.name = *projectName;

    s_activeProject = std::make_shared<Project>(spec);

    core::debug("loaded project from file");

    return s_activeProject;
}

auto Project::Save() -> std::expected<void, ProjectError> {
    auto result = WriteProjectFile();
    if (!result.has_value()) {
        return result;
    }

    core::debug("saved project");
    return {};
}

auto Project::GetProjectDirectory() const -> const std::filesystem::path & {
    core::expect(s_activeProject, "there must be an active project");
    return m_path;
}

auto Project::GetImagesDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "images";
}

auto Project::GetModelsDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "models";
}

auto Project::GetScenesDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "scenes";
}

auto Project::GetScriptsDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "scripts";
}

auto Project::GetShadersDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "shaders";
}

auto Project::GetActiveProject() -> std::shared_ptr<Project> {
    core::expect(s_activeProject, "there must be an active project");
    return s_activeProject;
}

auto Project::ConfigureProjectStructure() -> std::expected<void, ProjectError> {
    auto createDirectories = [](const std::filesystem::path &path) -> std::expected<void, ProjectError> {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);

        if (ec.value() != 0) {
            core::error("failed to create directory: {} with reason: {}", path.generic_string(), ec.message());
            return std::unexpected(ProjectError::FailedToCreateDirectory);
        }

        return {};
    };

    if (!std::filesystem::exists(m_path)) {
        core::trace("creating project directory at: {}", m_path.generic_string());
        auto result = createDirectories(m_path);
        if (!result.has_value()) {
            return result;
        }
    }

    if (!std::filesystem::is_directory(m_path)) {
        return std::unexpected(ProjectError::PathIsNotDirectory);
    }

    if (!std::filesystem::is_empty(m_path)) {
        return std::unexpected(ProjectError::DirectoryIsNotEmpty);
    }

    std::vector<std::filesystem::path> assetPaths = {
        GetImagesDirectory(), GetModelsDirectory(), GetScenesDirectory(), GetScriptsDirectory(), GetShadersDirectory(),
    };

    for (const auto &path : assetPaths) {
        core::trace("attempting to create: {}", path.generic_string());

        auto result = createDirectories(path);
        if (!result.has_value()) {
            return result;
        }
    }

    return {};
}

auto Project::WriteProjectFile() -> std::expected<void, ProjectError> {
    auto projectFilePath = m_path / "project.toml";

    toml::table projectConfig{
        {"name", m_name}
    };

    std::ofstream file{projectFilePath, std::ios::trunc};
    if (!file.is_open()) {
        return std::unexpected(ProjectError::FailedToOpenProjectFile);
    }
    file << projectConfig << std::endl;

    return {};
}

} // namespace muon
