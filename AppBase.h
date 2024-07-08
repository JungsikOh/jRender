#pragma once

#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GBuffer.h"
#include "GraphicsPSO.h"

namespace jRenderer {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

class AppBase {
  public:
    AppBase();
    virtual ~AppBase();

    int Run();
    float GetAspectRatio() const;

    virtual bool Initialize();
    virtual void UpdateGUI() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    virtual void OnMouseMove(int mouseX, int mouseY);
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void InitCubemaps(wstring basePath, wstring envFilename,
                      wstring specularFilename, wstring irradianceFilename,
                      wstring brdfFilename);
    void UpdateGlobalConstants(const Vector3 &eyeWorld, const Matrix &viewRow,
                               const Matrix &projRow);
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);
    void CreateDepthBuffers();
    void SetPipelineState(const GraphicsPSO &pso);
    bool UpdateMouseControl(const BoundingSphere &bs, Quaternion &q,
                            Vector3 &dragTranslation, Vector3 &pickPoint);

  protected: // 상속 받은 클래스에서도 접근 가능
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();
    void CreateBuffers();
    void SetMainViewport();
    void SetShadowViewport();

  public:
    int m_screenWidth;
    int m_screenHeight;
    HWND m_mainWindow;
    bool m_useMSAA = true;
    UINT m_numQualityLevels = 0;
    bool m_drawAsWire = false;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
    ComPtr<ID3D11ShaderResourceView> m_backBufferSRV;

    // 삼각형 레스터화 -> float(MSAA) -> resolved(No MSAA)
    ComPtr<ID3D11Texture2D> m_floatBuffer;
    ComPtr<ID3D11Texture2D> m_resolvedBuffer;
    ComPtr<ID3D11RenderTargetView> m_floatRTV;
    ComPtr<ID3D11RenderTargetView> m_resolvedRTV;
    ComPtr<ID3D11ShaderResourceView> m_resolvedSRV;

    // CubeMap
    ComPtr<ID3D11Texture2D> m_cubeMapStencilBuffer;
    ComPtr<ID3D11RenderTargetView> m_cubeMapStencilRTV;
    ComPtr<ID3D11ShaderResourceView> m_cubeMapStencilSRV;

    // CubeMap2
    ComPtr<ID3D11Texture2D> m_cubeMapBuffer;
    ComPtr<ID3D11RenderTargetView> m_cubeMapRTV;
    ComPtr<ID3D11ShaderResourceView> m_cubeMapSRV;

    // Depth Buffer
    ComPtr<ID3D11Texture2D> m_depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_depthOnlyDSV;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11ShaderResourceView> m_depthOnlySRV;

    D3D11_VIEWPORT m_screenViewport;

    // SSAO Noise
    ComPtr<ID3D11Texture2D> m_ssaoNoise;
    ComPtr<ID3D11ShaderResourceView> m_ssaoNoiseSRV;

    // SSAO
    ComPtr<ID3D11Texture2D> m_ssaoTex;
    ComPtr<ID3D11RenderTargetView> m_ssaoRTV;
    ComPtr<ID3D11ShaderResourceView> m_ssaoSRV;

    // SSAO Blur
    ComPtr<ID3D11Texture2D> m_ssaoBlurTex;
    ComPtr<ID3D11RenderTargetView> m_ssaoBlurRTV;
    ComPtr<ID3D11ShaderResourceView> m_ssaoBlurSRV;

        // Camera Class
        Camera m_camera;
    bool m_keyPressed[256] = {
        false,
    };

    bool m_leftButton = false;
    bool m_rightButton = false;
    bool m_dragStartFlag = false;

    float m_cursorNdcX = 0.0f;
    float m_cursorNdcY = 0.0f;
    bool m_selected = false; // 물체에 대한 인식

    // divide Constant Buffer for making different Passes.
    GlobalConstants m_globalConstsCPU;
    ComPtr<ID3D11Buffer> m_globalConstsGPU;

    // Shadow Mapping
    int m_shadowWidth = 2048;
    int m_shadowHeight = 2048;

    GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];

    // Shadow Buffer
    ComPtr<ID3D11Texture2D> m_shadowOnlyBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowOnlyDSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> m_shadowOnlySRVs[MAX_LIGHTS];

    // Shadow CubeMap
    ComPtr<ID3D11Texture2D> m_shadowCubeBuffers; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowCubeDSVs;
    ComPtr<ID3D11ShaderResourceView> m_shadowCubeSRVs;

    ShadowLightTransform m_pointLightTransformCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_pointLightTransformGPU[MAX_LIGHTS];

    // kernel Sample
    std::vector<Vector3> ssaoNoise; 
    Vector3kernelSampleConstants kernel;
    ComPtr<ID3D11Buffer> m_kernelSamplesGPU;

    // Post Processing
    PostEffectsConstants m_postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> m_postEffectsConstsGPU;

    // Common texutures
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

    // G-Buffer
    GBuffer m_gBuffer;

    bool m_lightRotate = false;
};

} // namespace jRenderer
