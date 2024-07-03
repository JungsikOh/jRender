#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // Comptr

#define SAFE_RELEASE(p)                                                        \
    {                                                                          \
        if ((p)) {                                                             \
            (p)->Release();                                                    \
            (p) = 0;                                                           \
        }                                                                      \
    }

namespace jRenderer {

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;
using namespace DirectX::SimpleMath;

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

class D3D11Utils {
  public:
    static Vector3 GetTangent(Vector3 edge1, Vector3 edge2, Vector2 deltaTex1,
                              Vector2 deltaTex2) {
        const float f =
            1.0f / (deltaTex1.x * deltaTex2.y - deltaTex1.y * deltaTex2.x);

        Vector3 tangent;
        tangent.x = f * (deltaTex2.y * edge1.x - deltaTex1.y * edge2.x);
        tangent.y = f * (deltaTex2.y * edge1.y - deltaTex1.y * edge2.y);
        tangent.z = f * (deltaTex2.y * edge1.z - deltaTex1.y * edge2.z);

        return tangent;
    }

    static void
    CreateComputeShader(ComPtr<ID3D11Device> &device, const wstring &filename,
                        ComPtr<ID3D11ComputeShader> &m_computeShader);

    static void CreateVertexShaderAndInputLayout(
        ComPtr<ID3D11Device> &device, const wstring &filename,
        const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
        ComPtr<ID3D11VertexShader> &m_vertexShader,
        ComPtr<ID3D11InputLayout> &m_inputLayout);

    static void CreateVertexShaderAndInputLayoutSum(
        ComPtr<ID3D11Device> &device, const wstring &filename,
        const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
        ComPtr<ID3D11VertexShader> &m_vertexShader,
        ComPtr<ID3D11InputLayout> &m_inputLayout);

    static void CreateHullShader(ComPtr<ID3D11Device> &device,
                                 const wstring &filename,
                                 ComPtr<ID3D11HullShader> &m_hullShader);

    static void CreateDomainShader(ComPtr<ID3D11Device> &device,
                                   const wstring &filename,
                                   ComPtr<ID3D11DomainShader> &m_domainShader);

    static void
    CreateGeometryShader(ComPtr<ID3D11Device> &device, const wstring &filename,
                         ComPtr<ID3D11GeometryShader> &m_geometryShader);

    static void CreatePixelShader(ComPtr<ID3D11Device> &device,
                                  const wstring &filename,
                                  ComPtr<ID3D11PixelShader> &m_pixelShader);

    static void CreatePixelShaderSum(ComPtr<ID3D11Device> &device,
                                     const wstring &filename,
                                     ComPtr<ID3D11PixelShader> &m_pixelShader);

    static void CreateIndexBuffer(ComPtr<ID3D11Device> &device,
                                  const vector<uint32_t> &incides,
                                  ComPtr<ID3D11Buffer> &indexBuffer);

    // desc -> subresource_data -> create
    template <typename T_VERTEX>
    static void CreateVertexBuffer(ComPtr<ID3D11Device> &device,
                                   const vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D11Buffer> &vertexBuffer) {
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 is no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(T_VERTEX);

        // initialize in MS Sample
        D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&bufferDesc, &vertexBufferData,
                                           vertexBuffer.GetAddressOf()));
    }

    static void CreateInstanceBuffer(ComPtr<ID3D11Device> &device,
                                     ComPtr<ID3D11Buffer> &instanceBuffer) {
        vector<Vector3> instances;
        instances.push_back(Vector3(-0.3f, 0.0f, 0.0f));
        instances.push_back(Vector3(0.5f, 0.5f, 0.5f));
        instances.push_back(Vector3(-0.1f, -0.2f, 0.3f));
        instances.push_back(Vector3(0.4f, 0.7f, -1.0f));

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = UINT(sizeof(Vector3) * instances.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        // initialize in MS Sample
        D3D11_SUBRESOURCE_DATA instanceBufferData = {0};
        instanceBufferData.pSysMem = instances.data();
        instanceBufferData.SysMemPitch = 0;
        instanceBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&bufferDesc, &instanceBufferData,
                                           instanceBuffer.GetAddressOf()));
    }

    // desc -> subresource_data -> create
    template <typename T_CONSTANT>
    static void CreateConstBuffer(ComPtr<ID3D11Device> &device,
                                  const T_CONSTANT &constantBufferData,
                                  ComPtr<ID3D11Buffer> &constantBuffer) {
        static_assert((sizeof(T_CONSTANT) % 16) == 0,
                      "Constant Buffer size must be 16-byte aligned");

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = sizeof(constantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC; // GPU read only, CPU write only
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &constantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&desc, &initData,
                                           constantBuffer.GetAddressOf()));
    }

    template <typename T_DATA>
    static void UpdateBuffer(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context,
                             const T_DATA &bufferData,
                             ComPtr<ID3D11Buffer> &buffer) {
        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        context->Unmap(buffer.Get(), NULL);
    }

    static void
    CreateTexture(ComPtr<ID3D11Device> &device,
                  ComPtr<ID3D11DeviceContext> &context,
                  const std::string filename, const bool usSRGB,
                  ComPtr<ID3D11Texture2D> &texture,
                  ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateMetallicRoughnessTexture(
        ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
        const std::string metallicFiilename,
        const std::string roughnessFilename, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &srv);

    static void
    CreateTextureArray(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::vector<std::string> filenames,
                       ComPtr<ID3D11Texture2D> &texture,
                       ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateTexture2D(
        ComPtr<ID3D11Device> &device,
        const vector<Vector3> &textureConstData, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &textureResourceView) {

        D3D11_TEXTURE2D_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = 4;
        txtDesc.Height = 4;
        txtDesc.MipLevels = 1; // 밉맵 레벨 최대
        txtDesc.ArraySize = 1;
        txtDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
        txtDesc.BindFlags =
            D3D11_BIND_SHADER_RESOURCE;
        txtDesc.MiscFlags = 0;
        txtDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = textureConstData.data();
        initData.SysMemPitch = txtDesc.Width * sizeof(Vector3);
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateTexture2D(&txtDesc, &initData, texture.GetAddressOf()));
        ThrowIfFailed(device->CreateShaderResourceView(
            texture.Get(), NULL, textureResourceView.GetAddressOf()));
    }

    static void CreateDDSTexture(ComPtr<ID3D11Device> &device,
                                 const wchar_t *filename, const bool isCubeMap,
                                 ComPtr<ID3D11ShaderResourceView> &texResView);

    // 텍스춰를 이미지 파일로 저장
    static void WriteToFile(ComPtr<ID3D11Device> &device,
                            ComPtr<ID3D11DeviceContext> &context,
                            ComPtr<ID3D11Texture2D> &textureToWrite,
                            const std::string filename);
};

} // namespace jRenderer