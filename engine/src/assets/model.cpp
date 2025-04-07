#include "muon/assets/model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace muon::assets {

    void loadModel(const std::filesystem::path &modelPath) {
        Assimp::Importer importer;

        auto flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;

        const aiScene *scene = importer.ReadFile(modelPath, flags);

        if (scene == nullptr) {
            // handle failure
        }

        if (scene->mNumMeshes <= 0) {
            // handle failure
        }

        aiMesh *mesh = scene->mMeshes[0];

        // load vertices and indices

    }

}
