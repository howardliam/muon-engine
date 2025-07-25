#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <string>

namespace muon::project {

enum class ProjectError {
    FailedToCreateDirectory,
    PathIsNotDirectory,
    DirectoryIsNotEmpty,
    FailedToOpenProjectFile,
    ProjectFileDoesNotExist,
    MalformedProjectFile,
};

class Project {
public:
    struct Spec {
        std::string name{"untitled"};
        std::filesystem::path path;
    };

public:
    Project(const Spec &spec);
    ~Project();

    static auto create(const Spec &spec) -> std::expected<std::shared_ptr<Project>, ProjectError>;
    static auto load(const std::filesystem::path &projectPath) -> std::expected<std::shared_ptr<Project>, ProjectError>;
    auto save() -> std::expected<void, ProjectError>;

public:
    auto getProjectDirectory() const -> const std::filesystem::path &;
    auto getImagesDirectory() const -> std::filesystem::path;
    auto getModelsDirectory() const -> std::filesystem::path;
    auto getScenesDirectory() const -> std::filesystem::path;
    auto getScriptsDirectory() const -> std::filesystem::path;
    auto getShadersDirectory() const -> std::filesystem::path;

    static auto getActiveProject() -> std::shared_ptr<Project>;

private:
    auto configureProjectStructure() -> std::expected<void, ProjectError>;
    auto writeProjectFile() -> std::expected<void, ProjectError>;

private:
    std::string m_name{"untitled"};
    std::filesystem::path m_path;

    static inline std::shared_ptr<Project> s_activeProject{nullptr};
};

} // namespace muon::project
