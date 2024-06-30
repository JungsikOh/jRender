#pragma once

#include <directxtk\SimpleMath.h>
#include <iostream>
#include <vector>

#include "D3D11Utils.h"
    
namespace jRenderer {

class GBuffer {
  public:
    GBuffer();
    ~GBuffer();

    HRESULT Init(ComPtr<ID3D11Device> &device, UINT width, UINT height);
    void Deinit();

    void PreRender(ComPtr<ID3D11DeviceContext> &context);
    void PostRender(ComPtr<ID3D11DeviceContext> &context);

    ID3D11Texture2D *GetColorTexture() { return m_colorSpecIntensityTex; }
    ID3D11DepthStencilView *GetDepthDSV() { return m_depthStencilDSV; }
    ID3D11DepthStencilView *GetDepthReadOnlyDSV() {
        return m_depthStencilReadOnlyDSV;
    }

    ID3D11ShaderResourceView *GetDepthView() { return m_depthStencilSRV; }
    ID3D11ShaderResourceView *GetColorView() { return m_colorSpecIntensitySRV; }
    ID3D11ShaderResourceView *GetNormalView() { return m_normalSRV; }
    ID3D11ShaderResourceView *GetSpecPowerView() { return m_specPowerSRV; }

  private:
    ID3D11Buffer *m_pGBufferUnpackCB;

    // GBuffer Textures
    ID3D11Texture2D *m_depthStencilTex;
    ID3D11Texture2D *m_colorSpecIntensityTex;
    ID3D11Texture2D *m_normalTex;
    ID3D11Texture2D *m_specPowerTex;

    // GBuffer DSVs
    ID3D11DepthStencilView *m_depthStencilDSV;
    ID3D11DepthStencilView *m_depthStencilReadOnlyDSV;
    // GBuffer RTVs
    ID3D11RenderTargetView *m_colorSpecIntensityRTV;
    ID3D11RenderTargetView *m_normalRTV;
    ID3D11RenderTargetView *m_specPowerRTV;

    // GBuffer SRVs
    ID3D11ShaderResourceView *m_depthStencilSRV;
    ID3D11ShaderResourceView *m_colorSpecIntensitySRV;
    ID3D11ShaderResourceView *m_normalSRV;
    ID3D11ShaderResourceView *m_specPowerSRV;

    ID3D11DepthStencilState *m_depthStencilState;
};

} // namespace jRenderer