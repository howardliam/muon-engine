#pragma once

#include "muon/core/errors.hpp"

#include <expected>
#include <filesystem>
#include <memory>
#include <string>

namespace muon {

class Project {
public:
    struct Spec {
        std::string name{"untitled"};
        std::filesystem::path path;
    };

public:
    Project(const Spec &spec);
    ~Project();

    static auto Create(const Spec &spec) -> std::expected<std::shared_ptr<Project>, ProjectError>;
    static auto Load(const std::filesystem::path &projectPath) -> std::expected<std::shared_ptr<Project>, ProjectError>;
    auto Save() -> std::expected<void, ProjectError>;

public:
    auto GetProjectDirectory() const -> const std::filesystem::path &;
    auto GetImagesDirectory() const -> std::filesystem::path;
    auto GetModelsDirectory() const -> std::filesystem::path;
    auto GetScenesDirectory() const -> std::filesystem::path;
    auto GetScriptsDirectory() const -> std::filesystem::path;
    auto GetShadersDirectory() const -> std::filesystem::path;

    static auto GetActiveProject() -> std::shared_ptr<Project>;

private:
    auto ConfigureProjectStructure() -> std::expected<void, ProjectError>;
    auto WriteProjectFile() -> std::expected<void, ProjectError>;

private:
    std::string m_name{"untitled"};
    std::filesystem::path m_path;

    static inline std::shared_ptr<Project> s_activeProject{nullptr};
};

} // namespace muon
