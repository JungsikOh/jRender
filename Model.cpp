#include "Model.h"

namespace jRenderer {

vector<MeshData> Model::ReadFromFile(std::string basePath, std::string filename,
                                     bool revertNormals) {

    using namespace DirectX;

    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);
    vector<MeshData> &meshes = modelLoader.meshes;

    // Normalize vertices (범위를 넘어서는 값들은 정리)
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    // 정규화 과정 [-1, 1] 범위로 바꿔줌으로써 NDC 규격을 맞추는 것이다.
    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float dl = XMMax(XMMax(dx, dy), dz);
    float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
          cz = (vmax.z + vmin.z) * 0.5f;

    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            v.position.x = (v.position.x - cx) / dl;
            v.position.y = (v.position.y - cy) / dl;
            v.position.z = (v.position.z - cz) / dl;
        }
    }

    return meshes;
}

Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename) {
    this->Initialize(device, context, basePath, filename);
}

Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::vector<MeshData> &meshes, int instanceFlag) {
    this->Initialize(device, context, meshes, instanceFlag);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::string &basePath,
                       const std::string &filename) {

    auto meshes = Model::ReadFromFile(basePath, filename);

    Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::vector<MeshData> &meshes, int instanceFlag) {

    // ConstantBuffer 만들기
    m_meshConstsCPU.world = Matrix();

    D3D11Utils::CreateConstBuffer(device, m_meshConstsCPU, m_meshConstsGPU);
    D3D11Utils::CreateConstBuffer(device, m_materialConstsCPU,
                                  m_materialConstsGPU);

    if (instanceFlag) {
        D3D11Utils::CreateConstBuffer(device, m_instancedConstsCPU,
                                      m_instancedConstsGPU);
        m_instancedConstsCPU.useInstancing = 1;
    }

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();
        D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
                                       newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.vertices.size());
        newMesh->strides = UINT(sizeof(Vertex));
        D3D11Utils::CreateIndexBuffer(device, meshData.indices,
                                      newMesh->indexBuffer);

        if (!meshData.albedoTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.albedoTextureFilename, true,
                newMesh->albedoTexture, newMesh->albedoSRV);
            m_materialConstsCPU.useAlbedoMap = true;
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.emissiveTextureFilename, true,
                newMesh->emissiveTexture, newMesh->emissiveSRV);
            m_materialConstsCPU.useEmissiveMap = true;
        }

        if (!meshData.normalTextureFilename.empty()) {
            D3D11Utils::CreateTexture(  
                device, context, meshData.normalTextureFilename, false,
                newMesh->normalTexture, newMesh->normalSRV);
            m_materialConstsCPU.useNormalMap = true;
        }

        if (!meshData.heightTextureFilename.empty()) {
            D3D11Utils::CreateTexture(
                device, context, meshData.heightTextureFilename, false,
                newMesh->heightTexture, newMesh->heightSRV);
            m_meshConstsCPU.useHeightMap = true;
        }

        if (!meshData.aoTextureFilename.empty()) {
            D3D11Utils::CreateTexture(device, context,
                                      meshData.aoTextureFilename, false,
                                      newMesh->aoTexture, newMesh->aoSRV);
            m_materialConstsCPU.useAOMap = true;
        }

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {
            D3D11Utils::CreateMetallicRoughnessTexture(
                device, context, meshData.metallicTextureFilename,
                meshData.roughnessTextureFilename,
                newMesh->metallicRoughnessTexture,
                newMesh->metallicRoughnessSRV);
        }

        if (!meshData.metallicTextureFilename.empty()) {
            m_materialConstsCPU.useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            m_materialConstsCPU.useRoughnessMap = true;
        }

        newMesh->vertexConstBuffer = m_meshConstsGPU;
        newMesh->pixelConstBuffer = m_materialConstsGPU;

        this->m_meshes.push_back(newMesh);
    }
}

void Model::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                  ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        D3D11Utils::UpdateBuffer(device, context, m_meshConstsCPU,
                                 m_meshConstsGPU);
        D3D11Utils::UpdateBuffer(device, context, m_materialConstsCPU,
                                 m_materialConstsGPU);
    }
    if (m_instancedConstsCPU.useInstancing) {
        D3D11Utils::UpdateBuffer(device, context, m_instancedConstsCPU,
                                 m_instancedConstsGPU);
    }
}

void Model::Render(ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        context->VSSetConstantBuffers(2, 1,
                                      m_instancedConstsGPU.GetAddressOf());
        for (const auto &mesh : m_meshes) {
            context->VSSetConstantBuffers(
                0, 1, mesh->vertexConstBuffer.GetAddressOf());
            context->PSSetConstantBuffers(
                0, 1, mesh->pixelConstBuffer.GetAddressOf());

            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());

            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};

            context->PSSetShaderResources(0, UINT(resViews.size()),
                                          resViews.data());

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->strides, &mesh->offsets);
            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                                      DXGI_FORMAT_R32_UINT, 0);
            if (!m_instancedConstsCPU.useInstancing)
                context->DrawIndexed(mesh->indexCount, 0, 0);
            else if (m_instancedConstsCPU.useInstancing)
                context->DrawIndexedInstanced(mesh->indexCount, m_instanceCount,
                                              0, 0, 0);
        }
    }
}

void Model::RenderScreen(ComPtr<ID3D11DeviceContext>& context) {
    ID3D11Buffer *nullBuffer = NULL;
    UINT stride = 0;
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
    context->Draw(6, 0);
}

void Model::RenderNormals(ComPtr<ID3D11DeviceContext> &context) {
    for (const auto &mesh : m_meshes) {
        context->GSSetConstantBuffers(0, 1, m_meshConstsGPU.GetAddressOf());
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &mesh->strides, &mesh->offsets);
        context->Draw(mesh->vertexCount, 0);
    }
}

void Model::UpdateWorldRow(const Matrix &worldRow) {
    this->m_worldRow = worldRow;
    this->m_worldITRow = worldRow;
    m_worldITRow.Translation(Vector3(0.0f));
    m_worldITRow = m_worldITRow.Invert().Transpose();

    m_meshConstsCPU.world = worldRow.Transpose();
    m_meshConstsCPU.worldIT = m_worldITRow.Transpose();
}

} // namespace jRenderer
