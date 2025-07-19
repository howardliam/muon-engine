#pragma once

namespace muon {

enum class ProjectError {
    FailedToCreateDirectory,
    PathIsNotDirectory,
    DirectoryIsNotEmpty,
    FailedToOpenProjectFile,
    ProjectFileDoesNotExist,
    MalformedProjectFile,
};

} // namespace muon
