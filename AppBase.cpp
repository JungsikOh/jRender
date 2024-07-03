#include "AppBase.h"

#include <algorithm>
#include <directxtk/SimpleMath.h>
#include <random>

#include "D3D11Utils.h"
#include "GraphicsCommon.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace jRenderer {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
// using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

AppBase *g_appBase =
    nullptr; // WINAPI 함수에 대한 접근성을 높이기 위한 전역 변수 선언

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_screenWidth(1280), m_screenHeight(720), m_mainWindow(0),
      m_screenViewport(D3D11_VIEWPORT()) {
    g_appBase = this;

    m_camera.SetAspectRatio(this->GetAspectRatio());
}

AppBase::~AppBase() {
    g_appBase = nullptr;

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_mainWindow);
}

float AppBase::GetAspectRatio() const {
    return float(m_screenWidth) / m_screenHeight;
}

int AppBase::Run() {

    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            // ImGui가 측정해주는 Framerate 출력
            ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            UpdateGUI(); // 추가적으로 사용할 GUI
            ImGui::End();
            ImGui::Render();

            Update(ImGui::GetIO().DeltaTime);

            Render(); // <- 중요: 우리가 구현한 렌더

            // GUI 렌더링
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // GUI 렌더링 후에 Present() 호출
            m_swapChain->Present(1, 0);
        }
    }

    return 0;
}

void AppBase::OnMouseMove(int mouseX, int mouseY) {

    // 마우스 커서의 위치를 NDC로 변환
    // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
    // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
    m_cursorNdcX = mouseX * 2.0f / m_screenWidth - 1.0f;
    m_cursorNdcY = -mouseY * 2.0f / m_screenHeight + 1.0f;

    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
    m_cursorNdcX = std::clamp(m_cursorNdcX, -1.0f, 1.0f);
    m_cursorNdcY = std::clamp(m_cursorNdcY, -1.0f, 1.0f);

    // 카메라 시점 회전
    m_camera.UpdateMouse(m_cursorNdcX, m_cursorNdcY);
}

LRESULT AppBase::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        // 화면 해상도가 바뀌면 SwapChain을 다시 생성
        if (m_swapChain) {

            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));

            m_backBufferRTV.Reset();
            m_swapChain->ResizeBuffers(0, // 현재 개수 유지
                                       (UINT)LOWORD(lParam), // 해상도 변경
                                       (UINT)HIWORD(lParam),
                                       DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                                       0);
            CreateBuffers();

            SetMainViewport();
            m_camera.SetAspectRatio(this->GetAspectRatio());
        }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        if (!m_leftButton) {
            m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        m_leftButton = true;
        break;
    case WM_LBUTTONUP:
        m_leftButton = false;
        break;
    case WM_RBUTTONDOWN:
        if (!m_rightButton) {
            m_dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        m_rightButton = true;
        break;
    case WM_RBUTTONUP:
        m_rightButton = false;
        break;
    case WM_KEYDOWN:
        m_keyPressed[wParam] = true;
        if (wParam == VK_ESCAPE) { // ESC키 종료
            DestroyWindow(hWnd);
        }
        if (wParam == VK_SPACE) {
            m_lightRotate = !m_lightRotate;
        }
        break;
    case WM_KEYUP:
        if (wParam == 'F') { // f키 일인칭 시점
            m_camera.m_useFirstPersonView = !m_camera.m_useFirstPersonView;
        }

        if (wParam == 'C') { // c키 화면 캡쳐
            ComPtr<ID3D11Texture2D> backBuffer;
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
            D3D11Utils::WriteToFile(m_device, m_context, backBuffer,
                                    "captured.png");
        }

        m_keyPressed[wParam] = false;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool AppBase::Initialize() {
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    if (!InitGUI())
        return false;

    // 콘솔창이 렌더링 창을 덮는 것을 방지
    SetForegroundWindow(m_mainWindow);

    return true;
}

bool AppBase::InitMainWindow() {
    WNDCLASSEX wcex = {sizeof(WNDCLASSEX),
                       CS_CLASSDC,
                       WndProc,
                       0L,
                       0L,
                       GetModuleHandle(NULL),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       L"jRenderer", // lpszClassName, L-string
                       NULL};
    if (!RegisterClassEx(&wcex)) {
        std::cout << "Not Register Window Class" << std::endl;
        return false;
    }

    RECT rc = {0, 0, m_screenWidth, m_screenHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
    m_mainWindow =
        CreateWindow(wcex.lpszClassName, L"jRenderer", WS_OVERLAPPEDWINDOW, 100,
                     100, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL,
                     wcex.hInstance, NULL);

    if (!m_mainWindow) {
        std::cout << "CreateWindow() isn't executed" << std::endl;
        return false;
    }

    ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_mainWindow);

    return true;
}

bool AppBase::InitDirect3D() {

    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format =
        DXGI_FORMAT_R8G8B8A8_UNORM; // 8bit. 즉, 0~255 사이의 값의 RGB를 가진다
                                    // 생각하면 된다. 그리고 float 0.0f~1.0f
                                    // 범위이다.
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;  // 분자
    sd.BufferDesc.RefreshRate.Denominator = 1; // 분모
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.OutputWindow = m_mainWindow;
    sd.Windowed = TRUE;
    // IDXGISwapChain::ResizeTarget을 호출하여 애플리케이션이 모드를 전환할 수
    //   있도록 하려면 이 플래그를 설정합니다
    //         .창에서 전체 화면 모드로 전환하면 디스플레이
    //         모드(또는 모니터 해상도)
    //             가 애플리케이션 창의 차원과 일치하도록 변경됩니다.
    // https://learn.microsoft.com/ko-kr/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect =
        DXGI_SWAP_EFFECT_FLIP_DISCARD; // After backbuffer swap, It is deleted.
    // _FLIP_ don't support MSAA
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(), &featureLevel, m_context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D Feature Level 11 unsupported." << std::endl;
        return false;
    }

    Graphics::InitCommonStates(m_device);

    CreateBuffers();

    SetMainViewport();

    // ConstBuffers for Common
    D3D11Utils::CreateConstBuffer(m_device, m_globalConstsCPU,
                                  m_globalConstsGPU);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        D3D11Utils::CreateConstBuffer(m_device, m_shadowGlobalConstsCPU[i],
                                      m_shadowGlobalConstsGPU[i]);
        D3D11Utils::CreateConstBuffer(m_device, m_pointLightTransformCPU[i],
                                      m_pointLightTransformGPU[i]);
    }

    // ConstBuffers for PostProcessing
    D3D11Utils::CreateConstBuffer(m_device, m_postEffectsConstsCPU,
                                  m_postEffectsConstsGPU);

    return true;
}

bool AppBase::InitGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_mainWindow)) {

        return false;
    }

    return true;
}

void AppBase::InitCubemaps(wstring basePath, wstring envFilename,
                           wstring specularFilename, wstring irradianceFilename,
                           wstring brdfFilename) {

    // BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
    D3D11Utils::CreateDDSTexture(m_device, (basePath + envFilename).c_str(),
                                 true, m_envSRV);
    D3D11Utils::CreateDDSTexture(
        m_device, (basePath + specularFilename).c_str(), true, m_specularSRV);
    D3D11Utils::CreateDDSTexture(m_device,
                                 (basePath + irradianceFilename).c_str(), true,
                                 m_irradianceSRV);
    D3D11Utils::CreateDDSTexture(m_device, (basePath + brdfFilename).c_str(),
                                 false, m_brdfSRV);
}

void AppBase::SetMainViewport() {

    // Set the viewport
    ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = float(m_screenWidth);
    m_screenViewport.Height = float(m_screenHeight);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &m_screenViewport);
}

void AppBase::SetShadowViewport() {

    // Set the viewport
    D3D11_VIEWPORT shadowViewport;
    ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
    shadowViewport.TopLeftX = 0;
    shadowViewport.TopLeftY = 0;
    shadowViewport.Width = float(m_shadowWidth);
    shadowViewport.Height = float(m_shadowHeight);
    shadowViewport.MinDepth = 0.0f;
    shadowViewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &shadowViewport);
}

void AppBase::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
    // 쉐이더와 일관성 유지 register(b1)
    m_context->VSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
    m_context->PSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
    m_context->GSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
}

void AppBase::SetPipelineState(const GraphicsPSO &pso) {
    m_context->VSSetShader(pso.m_vertexShader.Get(), 0, 0);
    m_context->PSSetShader(pso.m_pixelShader.Get(), 0, 0);
    // m_context->HSSetShader(pso.m_hullShader.Get(), 0, 0);
    // m_context->DSSetShader(pso.m_domainShader.Get(), 0, 0);
    m_context->GSSetShader(pso.m_geometryShader.Get(), 0, 0);
    m_context->IASetInputLayout(pso.m_inputLayout.Get());
    m_context->RSSetState(pso.m_rasterizerState.Get());
    // m_context->OMSetBlendState(pso.m_blendState.Get(), pso.m_blendFactor,
    //                            0xffffffff);
    m_context->OMSetDepthStencilState(pso.m_depthStencilState.Get(),
                                      pso.m_stencilRef);
    m_context->IASetPrimitiveTopology(pso.m_primitiveTopology);
}

void AppBase::CreateBuffers() {
    // resterlize -> MSAA -> backbuffer
    // 왜냐하면 지금은 postEffect 적용 안할거니까

    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        backBuffer.Get(), NULL, m_backBufferRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(
        backBuffer.Get(), NULL, m_backBufferSRV.GetAddressOf()));

    // 여기 주석을 해제하면 렌더링이 안됌 왜 그런걸까?

    //// set FLOAT MSAA RTV / SRV
    // ThrowIfFailed(m_device->CheckMultisampleQualityLevels(
    //     DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &m_numQualityLevels));

    // D3D11_TEXTURE2D_DESC desc;
    // backBuffer->GetDesc(&desc);
    // desc.MipLevels = desc.ArraySize = 1;
    // desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    // desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    // desc.Usage = D3D11_USAGE_DEFAULT; // It can copy from staging texture.
    // desc.MiscFlags = 0;
    // desc.CPUAccessFlags = 0;
    // if (m_useMSAA && m_numQualityLevels) {
    //     desc.SampleDesc.Count = 4;
    //     desc.SampleDesc.Quality = m_numQualityLevels - 1;
    // } else {
    //     desc.SampleDesc.Count = 1;
    //     desc.SampleDesc.Quality = 0;
    // }

    // ThrowIfFailed(
    //     m_device->CreateTexture2D(&desc, NULL,
    //     m_floatBuffer.GetAddressOf()));

    // ThrowIfFailed(m_device->CreateRenderTargetView(m_floatBuffer.Get(), NULL,
    //                                                m_floatRTV.GetAddressOf()));

    // NO MSAA RTV / SRV for PostPrecessing
    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.MipLevels = desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.Usage = D3D11_USAGE_DEFAULT; // It can copy from staging texture.
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_resolvedBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_resolvedBuffer.Get(), NULL, m_resolvedRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_resolvedBuffer.Get(), NULL, m_resolvedSRV.GetAddressOf()));

    // NO MSAA RTV / SRV for CubeMap
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_cubeMapBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(
        m_cubeMapBuffer.Get(), NULL, m_cubeMapRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_cubeMapBuffer.Get(), NULL, m_cubeMapSRV.GetAddressOf()));

    // NO MSAA RTV / SRV for CubeMap
    ThrowIfFailed(m_device->CreateTexture2D(
        &desc, NULL, m_cubeMapStencilBuffer.GetAddressOf()));
    ThrowIfFailed(
        m_device->CreateRenderTargetView(m_cubeMapStencilBuffer.Get(), NULL,
                                         m_cubeMapStencilRTV.GetAddressOf()));
    ThrowIfFailed(
        m_device->CreateShaderResourceView(m_cubeMapStencilBuffer.Get(), NULL,
                                           m_cubeMapStencilSRV.GetAddressOf()));

    // G-Buffer
    ThrowIfFailed(m_gBuffer.Init(m_device, m_screenWidth, m_screenHeight));

    // ssaoNoise
    // 0 부터 1 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    std::vector<Vector3> ssaoNoise;
    for (int i = 0; i < 16; i++) {
        Vector3 tmp = Vector3(randomFloats(generator) * 2.0f - 1.0f,
                              randomFloats(generator) * 2.0f - 1.0f, 0.0f);
        ssaoNoise.push_back(tmp);
    }
    Vector3kernelSampleConstants kernel;
    for (UINT i = 0; i < MAX_SAMPLES; ++i) {
        Vector3 _sample = Vector3(randomFloats(generator) * 2.0f - 1.0f,
                                  randomFloats(generator) * 2.0f - 1.0f,
                                  randomFloats(generator));
        kernel.samples[i] = _sample;
    }
    D3D11Utils::CreateConstBuffer(m_device, kernel, m_kernelSamplesGPU);
    D3D11Utils::CreateTexture2D(m_device, ssaoNoise, m_ssaoNoise,
                                m_ssaoNoiseSRV);

    // SSAO
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_ssaoTex.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_ssaoTex.Get(), NULL,
                                                   m_ssaoRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(m_ssaoTex.Get(), NULL,
                                                     m_ssaoSRV.GetAddressOf()));

    // SSAO Blur
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, NULL, m_ssaoBlurTex.GetAddressOf()));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_ssaoBlurTex.Get(), NULL,
                                                   m_ssaoBlurRTV.GetAddressOf()));
    ThrowIfFailed(m_device->CreateShaderResourceView(m_ssaoBlurTex.Get(), NULL,
                                                     m_ssaoBlurSRV.GetAddressOf()));

    CreateDepthBuffers();
}

// 여러 물체들이 공통적으료 사용하는 Const 업데이트
void AppBase::UpdateGlobalConstants(const Vector3 &eyeWorld,
                                    const Matrix &viewRow,
                                    const Matrix &projRow) {

    m_globalConstsCPU.eyeWorld = eyeWorld;
    m_globalConstsCPU.view = viewRow.Transpose();
    m_globalConstsCPU.proj = projRow.Transpose();
    m_globalConstsCPU.invProj = projRow.Invert().Transpose();
    m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용
    m_globalConstsCPU.invViewProj = m_globalConstsCPU.viewProj.Invert();

    // m_reflectGlobalConstsCPU = m_globalConstsCPU;
    // memcpy(&m_reflectGlobalConstsCPU, &m_globalConstsCPU,
    //        sizeof(m_globalConstsCPU));
    // m_reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
    // m_reflectGlobalConstsCPU.viewProj = (refl * viewRow *
    // projRow).Transpose();
    //// 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    // m_reflectGlobalConstsCPU.invViewProj =
    //     m_reflectGlobalConstsCPU.viewProj.Invert();

    D3D11Utils::UpdateBuffer(m_device, m_context, m_globalConstsCPU,
                             m_globalConstsGPU);
    // D3D11Utils::UpdateBuffer(m_device, m_context, m_reflectGlobalConstsCPU,
    //                          m_reflectGlobalConstsGPU);
}

void AppBase::CreateDepthBuffers() {

    // DepthStencilView
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_screenWidth;
    desc.Height = m_screenHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if (m_useMSAA && m_numQualityLevels) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = m_numQualityLevels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    ThrowIfFailed(
        m_device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf()));
    ThrowIfFailed(m_device->CreateDepthStencilView(
        depthStencilBuffer.Get(), NULL, m_depthStencilView.GetAddressOf()));

    // Depth 전용
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(m_device->CreateTexture2D(&desc, NULL,
                                            m_depthOnlyBuffer.GetAddressOf()));

    // Shadow 전용
    desc.Width = m_shadowWidth;
    desc.Height = m_shadowHeight;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateTexture2D(
            &desc, NULL, m_shadowOnlyBuffers[i].GetAddressOf()));
    }
    // shadowCubeMap
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.ArraySize = 6;
    ThrowIfFailed(m_device->CreateTexture2D(
        &desc, NULL, m_shadowCubeBuffers.GetAddressOf()));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(m_device->CreateDepthStencilView(
        m_depthOnlyBuffer.Get(), &dsvDesc, m_depthOnlyDSV.GetAddressOf()));

    // Shadow 전용
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateDepthStencilView(
            m_shadowOnlyBuffers[i].Get(), &dsvDesc,
            m_shadowOnlyDSVs[i].GetAddressOf()));
    }

    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dsvDesc.Texture2DArray.ArraySize = 6;
    dsvDesc.Texture2DArray.FirstArraySlice = 0;
    dsvDesc.Texture2DArray.MipSlice = 0;
    dsvDesc.Flags = 0;
    ThrowIfFailed(m_device->CreateDepthStencilView(
        m_shadowCubeBuffers.Get(), &dsvDesc, m_shadowCubeDSVs.GetAddressOf()));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_depthOnlyBuffer.Get(), &srvDesc, m_depthOnlySRV.GetAddressOf()));

    // Shadow 전용
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        ThrowIfFailed(m_device->CreateShaderResourceView(
            m_shadowOnlyBuffers[i].Get(), &srvDesc,
            m_shadowOnlySRVs[i].GetAddressOf()));
    }

    // shadowCube 전용
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;
    ThrowIfFailed(m_device->CreateShaderResourceView(
        m_shadowCubeBuffers.Get(), &srvDesc, m_shadowCubeSRVs.GetAddressOf()));
}

} // namespace jRenderer