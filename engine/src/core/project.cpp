#include "muon/core/project.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "yaml-cpp/emittermanip.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/yaml.h>

namespace muon {

Project::Project(const Spec &spec) : m_name(spec.name), m_directory(spec.directory) {}

Project::~Project() {
    auto success = Save();
    MU_CORE_ASSERT(success, "failed to save project file to disk");
}

auto Project::Create(const Spec &spec) -> std::shared_ptr<Project> {
    s_activeProject = std::make_shared<Project>(spec);
    s_activeProject->ConfigureProjectStructure();
    s_activeProject->CreateProjectFile();

    MU_CORE_DEBUG("created new project");

    return s_activeProject;
}

auto Project::Load(const std::filesystem::path &projectFile) -> std::shared_ptr<Project> {
    Spec spec{};
    try {
        YAML::Node projectYaml = YAML::LoadFile(projectFile);
        spec.name = projectYaml["name"].as<std::string>();
        spec.directory = projectYaml["directory"].as<std::string>();
    } catch (const std::exception &e) { MU_CORE_ASSERT("failed to load project file"); }

    s_activeProject = std::make_shared<Project>(spec);
    s_activeProject->m_projectFilePath = projectFile;

    MU_CORE_DEBUG("loaded project from file");

    return s_activeProject;
}

auto Project::Save() -> bool {
    MU_CORE_DEBUG("saved project");
    return true;
}

auto Project::GetProjectDirectory() const -> const std::filesystem::path & { return m_directory; }
auto Project::GetImagesDirectory() const -> std::filesystem::path { return m_directory / "images"; }
auto Project::GetModelsDirectory() const -> std::filesystem::path { return m_directory / "models"; }
auto Project::GetScenesDirectory() const -> std::filesystem::path { return m_directory / "scenes"; }
auto Project::GetScriptsDirectory() const -> std::filesystem::path { return m_directory / "scripts"; }
auto Project::GetShadersDirectory() const -> std::filesystem::path { return m_directory / "shaders"; }

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

    if (!std::filesystem::exists(m_directory)) {
        MU_CORE_TRACE("creating working directory at: {}", m_directory.generic_string());
        auto success = createDirectories(m_directory);
        MU_CORE_ASSERT(success, "failed to create working directory");
    }

    MU_CORE_ASSERT(std::filesystem::is_directory(m_directory), "path to working directory must be a directory");
    MU_CORE_ASSERT(std::filesystem::is_empty(m_directory), "working directory must be empty on initialization");

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

auto Project::CreateProjectFile() -> void {
    m_projectFilePath = m_directory / (m_name + ".yaml");
    MU_CORE_ASSERT(!std::filesystem::exists(m_projectFilePath), "path is not empty");

    std::ofstream file{m_projectFilePath};
    MU_CORE_ASSERT(file.is_open(), "failed to open project file");

    YAML::Emitter emitter;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "name";
    emitter << YAML::Value << m_name;
    emitter << YAML::Key << "directory";
    emitter << YAML::Value << m_directory.generic_string();
    emitter << YAML::EndMap;

    file.write(emitter.c_str(), emitter.size());
}

} // namespace muon
