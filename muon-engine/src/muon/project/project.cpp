#include "muon/project/project.hpp"

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

namespace muon::project {

Project::Project(const Spec &spec) : m_name(spec.name), m_path(spec.path) {}

Project::~Project() {
    auto success = save();
    core::expect(success, "failed to save project file to disk");
}

auto Project::create(const Spec &spec) -> std::expected<std::shared_ptr<Project>, ProjectError> {
    s_activeProject = std::make_shared<Project>(spec);

    auto result = s_activeProject->configureProjectStructure();
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    result = s_activeProject->writeProjectFile();
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    core::debug("created new project");

    return s_activeProject;
}

auto Project::load(const std::filesystem::path &projectPath) -> std::expected<std::shared_ptr<Project>, ProjectError> {
    if (!std::filesystem::exists(projectPath)) {
        core::trace("no project exists, attempting to create");
        Spec spec{};
        spec.path = projectPath;
        return create(spec);
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

auto Project::save() -> std::expected<void, ProjectError> {
    auto result = writeProjectFile();
    if (!result.has_value()) {
        return result;
    }

    core::debug("saved project");
    return {};
}

auto Project::getProjectDirectory() const -> const std::filesystem::path & {
    core::expect(s_activeProject, "there must be an active project");
    return m_path;
}

auto Project::getImagesDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "images";
}

auto Project::getModelsDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "models";
}

auto Project::getScenesDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "scenes";
}

auto Project::getScriptsDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "scripts";
}

auto Project::getShadersDirectory() const -> std::filesystem::path {
    core::expect(s_activeProject, "there must be an active project");
    return m_path / "shaders";
}

auto Project::getActiveProject() -> std::shared_ptr<Project> {
    core::expect(s_activeProject, "there must be an active project");
    return s_activeProject;
}

auto Project::configureProjectStructure() -> std::expected<void, ProjectError> {
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
        getImagesDirectory(), getModelsDirectory(), getScenesDirectory(), getScriptsDirectory(), getShadersDirectory(),
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

auto Project::writeProjectFile() -> std::expected<void, ProjectError> {
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

} // namespace muon::project
