#include "muon/assets/model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <print>

namespace muon::assets {

    std::optional<aiMesh *> loadModel(const std::filesystem::path &modelPath) {
        Assimp::Importer importer;

        auto flags = aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;
        // auto flags = 0;

        const aiScene *scene = importer.ReadFile(modelPath, flags);

        if (scene == nullptr) {
            std::println("no scene");
            return {};
        }

        if (scene->mNumMeshes <= 0) {
            std::println("no meshes");
            return {};
        }

        aiMesh *mesh = scene->mMeshes[0];

        return mesh;
    }

}
