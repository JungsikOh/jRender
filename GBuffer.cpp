#include "GBuffer.h"
#include "Camera.h"

namespace jRenderer {

GBuffer::GBuffer()
    : m_pGBufferUnpackCB(NULL), m_depthStencilTex(NULL),
      m_colorSpecIntensityTex(NULL), m_normalTex(NULL), m_specPowerTex(NULL),
      m_depthStencilDSV(NULL), m_depthStencilReadOnlyDSV(NULL),
      m_colorSpecIntensityRTV(NULL), m_normalRTV(NULL), m_specPowerRTV(NULL),
      m_depthStencilSRV(NULL), m_colorSpecIntensitySRV(NULL), m_normalSRV(NULL),
      m_specPowerSRV(NULL), m_depthStencilState(NULL) {}

GBuffer::~GBuffer() {}

HRESULT GBuffer::Init(ComPtr<ID3D11Device> &g_pDevice, UINT width, UINT height) {
    HRESULT hr;

    Deinit(); // Clear the previous targets

    // Texture formats
    static const DXGI_FORMAT depthStencilTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
    static const DXGI_FORMAT basicColorTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT normalTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT specPowTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Render View formats
    static const DXGI_FORMAT depthStencilRenderViewFormat =
        DXGI_FORMAT_D24_UNORM_S8_UINT;
    static const DXGI_FORMAT basicColorRenderViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT normalRenderViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT specPowRenderViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    // Resource view formats
    static const DXGI_FORMAT depthStencilResourceViewFormat =
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    static const DXGI_FORMAT basicColorResourceViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT normalResourceViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT specPowResourceViewFormat =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    // Allocate the depth stencil target
    D3D11_TEXTURE2D_DESC dtd = {
        width,               // UINT Width;
        height,              // UINT Height;
        1,                   // UINT MipLevels;
        1,                   // UINT ArraySize;
        DXGI_FORMAT_UNKNOWN, // DXGI_FORMAT Format;
        1,                   // DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT, // D3D11_USAGE Usage;
        D3D11_BIND_DEPTH_STENCIL |
            D3D11_BIND_SHADER_RESOURCE, // UINT BindFlags;
        0,                              // UINT CPUAccessFlags;
        0                               // UINT MiscFlags;
    };
    dtd.Format = depthStencilTextureFormat;
    ThrowIfFailed(g_pDevice->CreateTexture2D(&dtd, NULL, &m_depthStencilTex));

    // Allocate the base color with specular intensity target
    dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    dtd.Format = basicColorTextureFormat; 
    ThrowIfFailed(
        g_pDevice->CreateTexture2D(&dtd, NULL, &m_colorSpecIntensityTex));

    // Allocate the base color with specular intensity target
    dtd.Format = normalTextureFormat;
    ThrowIfFailed(g_pDevice->CreateTexture2D(&dtd, NULL, &m_normalTex));

    // Allocate the specular power target
    dtd.Format = specPowTextureFormat;
    ThrowIfFailed(g_pDevice->CreateTexture2D(&dtd, NULL, &m_specPowerTex));

    // Create the render target views
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {depthStencilRenderViewFormat,
                                          D3D11_DSV_DIMENSION_TEXTURE2D, 0};
    ThrowIfFailed(g_pDevice->CreateDepthStencilView(m_depthStencilTex, &dsvd,
                                                    &m_depthStencilDSV));

    dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
    ThrowIfFailed(g_pDevice->CreateDepthStencilView(
        m_depthStencilTex, &dsvd, &m_depthStencilReadOnlyDSV));

    D3D11_RENDER_TARGET_VIEW_DESC rtsvd = {basicColorRenderViewFormat,
                                           D3D11_RTV_DIMENSION_TEXTURE2D};

    ThrowIfFailed(g_pDevice->CreateRenderTargetView(
        m_colorSpecIntensityTex, &rtsvd, &m_colorSpecIntensityRTV));

    rtsvd.Format = normalRenderViewFormat;
    ThrowIfFailed(
        g_pDevice->CreateRenderTargetView(m_normalTex, &rtsvd, &m_normalRTV));

    rtsvd.Format = specPowRenderViewFormat;
    ThrowIfFailed(g_pDevice->CreateRenderTargetView(m_specPowerTex, &rtsvd,
                                                    &m_specPowerRTV));

    // Create the resource views
    D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
        depthStencilResourceViewFormat, D3D11_SRV_DIMENSION_TEXTURE2D, 0, 0};
    dsrvd.Texture2D.MipLevels = 1;
    ThrowIfFailed(g_pDevice->CreateShaderResourceView(m_depthStencilTex, &dsrvd,
                                                      &m_depthStencilSRV));

    dsrvd.Format = basicColorResourceViewFormat;
    ThrowIfFailed(g_pDevice->CreateShaderResourceView(
        m_colorSpecIntensityTex, &dsrvd, &m_colorSpecIntensitySRV)); 

    dsrvd.Format = normalResourceViewFormat;
    ThrowIfFailed(
        g_pDevice->CreateShaderResourceView(m_normalTex, &dsrvd, &m_normalSRV));

    dsrvd.Format = specPowResourceViewFormat;
    ThrowIfFailed(g_pDevice->CreateShaderResourceView(m_specPowerTex, &dsrvd,
                                                 &m_specPowerSRV));

    D3D11_DEPTH_STENCIL_DESC descDepth;
    descDepth.DepthEnable = TRUE;
    descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    descDepth.DepthFunc = D3D11_COMPARISON_LESS;
    descDepth.StencilEnable = TRUE;
    descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = {
        D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS};
    descDepth.FrontFace = stencilMarkOp;
    descDepth.BackFace = stencilMarkOp;
    ThrowIfFailed(
        g_pDevice->CreateDepthStencilState(&descDepth, &m_depthStencilState));

    return S_OK;
}

void GBuffer::Deinit() {
    SAFE_RELEASE(m_pGBufferUnpackCB);

    // Clear all allocated targets
    SAFE_RELEASE(m_depthStencilTex);
    SAFE_RELEASE(m_colorSpecIntensityTex);
    SAFE_RELEASE(m_normalTex);
    SAFE_RELEASE(m_specPowerTex);

    // Clear all views
    SAFE_RELEASE(m_depthStencilDSV);
    SAFE_RELEASE(m_depthStencilReadOnlyDSV);
    SAFE_RELEASE(m_colorSpecIntensityRTV);
    SAFE_RELEASE(m_normalRTV);
    SAFE_RELEASE(m_specPowerRTV);
    SAFE_RELEASE(m_depthStencilSRV);
    SAFE_RELEASE(m_colorSpecIntensitySRV);
    SAFE_RELEASE(m_normalSRV);
    SAFE_RELEASE(m_specPowerSRV);

    // Clear the depth stencil state
    SAFE_RELEASE(m_depthStencilState);
}

void GBuffer::PreRender(ComPtr<ID3D11DeviceContext>& context) {
    context->ClearDepthStencilView(
        m_depthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

    // you only need to do this if your scene doesn't cover the whole visible area
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    context->ClearRenderTargetView(m_colorSpecIntensityRTV, clearColor);
    context->ClearRenderTargetView(m_normalRTV, clearColor);
    context->ClearRenderTargetView(m_specPowerRTV, clearColor);
     
    ID3D11RenderTargetView *RTVs[3] = {m_colorSpecIntensityRTV, m_normalRTV,
                                     m_specPowerRTV}; 
    context->OMSetRenderTargets(3, RTVs, m_depthStencilDSV);
    context->OMSetDepthStencilState(m_depthStencilState, 1);
}

void GBuffer::PostRender(ComPtr<ID3D11DeviceContext>& context) {
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->OMSetRenderTargets(3, NULL, m_depthStencilReadOnlyDSV);
}

} // namespace jRenderer
