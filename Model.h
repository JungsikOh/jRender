#pragma once

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "Mesh.h"
#include "MeshData.h"
#include "ModelLoader.h"

// ref
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

namespace jRenderer {

class Model {
  public:
    Model() {}
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const std::string &basePath, const std::string &filename);
    Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const std::vector<MeshData> &meshes, int instanceFlag);
      
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::string &basePath, const std::string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<MeshData> &meshes, int instanceFlag = 0);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11DeviceContext> &context);

    void RenderScreen(ComPtr<ID3D11DeviceContext> &context);

    void RenderNormals(ComPtr<ID3D11DeviceContext> &context);

    void UpdateWorldRow(const Matrix &worldRow);        

    static vector<MeshData> ReadFromFile(std::string basePath, std::string filename,
                                  bool revertNormals = false);

  public:
    Matrix m_worldRow = Matrix();   // Model(Object) To World
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    MeshConstants m_meshConstsCPU;
    InstancedConsts m_instancedConstsCPU;
    MaterialConstants m_materialConstsCPU;

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true;

    int m_instanceCount = MAX_INSTANCE;

    std::vector<shared_ptr<Mesh>> m_meshes;

  private:
    ComPtr<ID3D11Buffer> m_meshConstsGPU;
    ComPtr<ID3D11Buffer> m_instancedConstsGPU;
    ComPtr<ID3D11Buffer> m_materialConstsGPU;
};

} // namespace jRenderer