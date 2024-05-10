#pragma once

// vcpkg install assimp:x64-windows
// add NOMINMAX in Preprocessor definitions
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "MeshData.h"
#include "Vertex.h"

namespace jRenderer {

class ModelLoader {
  public:
    void Load(std::string basePath, std::string filename, bool revertNormals);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);

    MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

    std::string ReadFilename(aiMaterial *material, aiTextureType type);

    void UpdateTangents();

  public:
    std::string basePath;
    std::vector<MeshData> meshes;
    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;
};

} // namespace jRenederer