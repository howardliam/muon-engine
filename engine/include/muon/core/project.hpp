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

    static auto Create(const Spec &spec) -> std::shared_ptr<Project>;
    static auto Load(const std::filesystem::path &projectPath) -> std::shared_ptr<Project>;
    [[nodiscard]] auto Save() -> bool;

public:
    [[nodiscard]] auto GetProjectDirectory() const -> const std::filesystem::path &;
    [[nodiscard]] auto GetImagesDirectory() const -> std::filesystem::path;
    [[nodiscard]] auto GetModelsDirectory() const -> std::filesystem::path;
    [[nodiscard]] auto GetScenesDirectory() const -> std::filesystem::path;
    [[nodiscard]] auto GetScriptsDirectory() const -> std::filesystem::path;
    [[nodiscard]] auto GetShadersDirectory() const -> std::filesystem::path;

    [[nodiscard]] static auto GetActiveProject() -> std::shared_ptr<Project>;

private:
    auto ConfigureProjectStructure() -> void;
    [[nodiscard]] auto WriteProjectFile() -> std::expected<void, FileSystemError>;

private:
    std::string m_name{"untitled"};
    std::filesystem::path m_path;

    static inline std::shared_ptr<Project> s_activeProject{nullptr};
};

} // namespace muon
