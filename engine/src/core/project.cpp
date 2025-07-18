#include "muon/core/project.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/errors.hpp"
#include "muon/core/log.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

namespace muon {

Project::Project(const Spec &spec) : m_name(spec.name), m_path(spec.path) {}

Project::~Project() {
    auto success = Save();
    MU_CORE_ASSERT(success, "failed to save project file to disk");
}

auto Project::Create(const Spec &spec) -> std::shared_ptr<Project> {
    s_activeProject = std::make_shared<Project>(spec);
    s_activeProject->ConfigureProjectStructure();
    auto result = s_activeProject->WriteProjectFile();
    if (!result.has_value()) {
        switch (result.error()) {
            case FileSystemError::BadFile:
                MU_CORE_ERROR("failed to write project file");
                break;
        }
    }

    MU_CORE_DEBUG("created new project");

    return s_activeProject;
}

auto Project::Load(const std::filesystem::path &projectPath) -> std::shared_ptr<Project> {
    auto config = toml::parse_file((projectPath / "project.toml").c_str());

    auto projectName = config["name"].value<std::string_view>();
    MU_CORE_ASSERT(projectName.has_value(), "`project.toml` must have a name field");

    Spec spec{};
    spec.path = projectPath;
    spec.name = *projectName;

    s_activeProject = std::make_shared<Project>(spec);

    MU_CORE_DEBUG("loaded project from file");

    return s_activeProject;
}

auto Project::Save() -> bool {
    auto result = WriteProjectFile();
    if (!result.has_value()) {
        switch (result.error()) {
            case FileSystemError::BadFile:
                MU_CORE_ERROR("failed to write project file");
                break;
        }
    }

    MU_CORE_DEBUG("saved project");
    return true;
}

auto Project::GetProjectDirectory() const -> const std::filesystem::path & { return m_path; }
auto Project::GetImagesDirectory() const -> std::filesystem::path { return m_path / "images"; }
auto Project::GetModelsDirectory() const -> std::filesystem::path { return m_path / "models"; }
auto Project::GetScenesDirectory() const -> std::filesystem::path { return m_path / "scenes"; }
auto Project::GetScriptsDirectory() const -> std::filesystem::path { return m_path / "scripts"; }
auto Project::GetShadersDirectory() const -> std::filesystem::path { return m_path / "shaders"; }

auto Project::GetActiveProject() -> std::shared_ptr<Project> { return s_activeProject; }

auto Project::ConfigureProjectStructure() -> void {
    auto createDirectories = [](const std::filesystem::path &path) -> bool {
        bool success = false;
        try {
            success = std::filesystem::create_directories(path);
        } catch (const std::exception &e) {
            MU_CORE_ERROR("failed to create directory: {} with reason: {}", path.generic_string(), e.what());
            return success;
        }
        return success;
    };

    if (!std::filesystem::exists(m_path)) {
        MU_CORE_TRACE("creating working directory at: {}", m_path.generic_string());
        auto success = createDirectories(m_path);
        MU_CORE_ASSERT(success, "failed to create working directory");
    }

    MU_CORE_ASSERT(std::filesystem::is_directory(m_path), "path to working directory must be a directory");
    MU_CORE_ASSERT(std::filesystem::is_empty(m_path), "working directory must be empty on initialization");

    std::vector<std::filesystem::path> assetPaths = {
        GetImagesDirectory(), GetModelsDirectory(), GetScenesDirectory(), GetScriptsDirectory(), GetShadersDirectory(),
    };

    for (const auto &path : assetPaths) {
        MU_CORE_TRACE("attempting to create: {}", path.generic_string());

        if (std::filesystem::exists(path)) {
            MU_CORE_ASSERT(std::filesystem::is_directory(path), "project subdirectory must be a directory");
            MU_CORE_TRACE("{} already exists, skipping", path.generic_string());
            continue;
        }

        auto success = createDirectories(path);
        MU_CORE_ASSERT(success, "failed to create project subdirectory");
    }
}

auto Project::WriteProjectFile() -> std::expected<void, FileSystemError> {
    auto projectFilePath = m_path / "project.toml";

    toml::table projectConfig{
        {"name", m_name}
    };

    std::ofstream file{projectFilePath, std::ios::trunc};
    if (!file.is_open()) {
        return std::unexpected(FileSystemError::BadFile);
    }
    file << projectConfig;

    return {};
}

} // namespace muon
