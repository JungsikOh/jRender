#include "GraphicsPSO.h"

namespace jRenderer {

namespace Graphics {

// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> linearClampSS; 
extern ComPtr<ID3D11SamplerState> shadowPointSS;
extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
extern ComPtr<ID3D11RasterizerState> solidRS;
extern ComPtr<ID3D11RasterizerState> solidCCWRS; // Counter-ClockWise
extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCCWRS;
extern ComPtr<ID3D11RasterizerState> depthOnlyRS;
extern ComPtr<ID3D11RasterizerState> postProcessingRS;

// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS; // 일반적으로 그리기
extern ComPtr<ID3D11DepthStencilState> maskDSS; // 스텐실버퍼에 표시
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // 스텐실 표시된 곳만

// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11VertexShader> instancedVS;
extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11VertexShader> samplingVS;
extern ComPtr<ID3D11VertexShader> normalVS;
extern ComPtr<ID3D11VertexShader> depthOnlyVS;
extern ComPtr<ID3D11VertexShader> shadowCubeMapVS;
extern ComPtr<ID3D11VertexShader> postEffectsVS;
extern ComPtr<ID3D11VertexShader> ssaoVS;
extern ComPtr<ID3D11VertexShader> ssaoBlurVS;

extern ComPtr<ID3D11PixelShader> basicPS;
extern ComPtr<ID3D11PixelShader> skyboxPS;
extern ComPtr<ID3D11PixelShader> combinePS;
extern ComPtr<ID3D11PixelShader> bloomDownPS;
extern ComPtr<ID3D11PixelShader> bloomUpPS;
extern ComPtr<ID3D11PixelShader> normalPS;
extern ComPtr<ID3D11PixelShader> depthOnlyPS;
extern ComPtr<ID3D11PixelShader> shadowCubeMapPS;
extern ComPtr<ID3D11PixelShader> postEffectsPS;
extern ComPtr<ID3D11PixelShader> ssaoPS;
extern ComPtr<ID3D11PixelShader> ssaoBlurPS;

extern ComPtr<ID3D11GeometryShader> normalGS;
extern ComPtr<ID3D11GeometryShader> shadowCubeMapGS;

//Gbuffer
extern ComPtr<ID3D11VertexShader> gBufferVS;
extern ComPtr<ID3D11PixelShader> gBufferPS;
extern ComPtr<ID3D11PixelShader> deferredLightingPS;

// Render Pass
extern ComPtr<ID3D11VertexShader> ScreenVS;
extern ComPtr<ID3D11PixelShader> renderPassPS;

// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> instancedIL;
extern ComPtr<ID3D11InputLayout> samplingIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;
extern ComPtr<ID3D11InputLayout> postProcessingIL;
extern ComPtr<ID3D11InputLayout> nullIL;

// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;

// Graphics Pipeline States
extern GraphicsPSO defaultSolidPSO;
extern GraphicsPSO defaultWirePSO;
extern GraphicsPSO instanceSolidPSO;
extern GraphicsPSO stencilMaskPSO;
extern GraphicsPSO reflectSolidPSO;
extern GraphicsPSO reflectWirePSO;
extern GraphicsPSO mirrorBlendSolidPSO;
extern GraphicsPSO mirrorBlendWirePSO;
extern GraphicsPSO skyboxSolidPSO;
extern GraphicsPSO skyboxWirePSO;
extern GraphicsPSO reflectSkyboxSolidPSO;
extern GraphicsPSO reflectSkyboxWirePSO;
extern GraphicsPSO normalsPSO;
extern GraphicsPSO depthOnlyPSO;
extern GraphicsPSO shadowCubeMapPSO;
extern GraphicsPSO deferredLightingPSO;
extern GraphicsPSO postEffectsPSO;
extern GraphicsPSO postProcessingPSO;
extern GraphicsPSO gBufferPSO;
extern GraphicsPSO renderPassPSO;
extern GraphicsPSO ssaoPSO;
extern GraphicsPSO ssaoBlurPSO;

void InitCommonStates(ComPtr<ID3D11Device> &device);
void ShutdownStates();

// 내부적으로 InitCommonStates()에서 사용
void InitSamplers(ComPtr<ID3D11Device> &device);
void InitRasterizerStates(ComPtr<ID3D11Device> &device);
void InitBlendStates(ComPtr<ID3D11Device> &device);
void InitDepthStencilStates(ComPtr<ID3D11Device> &device);
void InitPipelineStates(ComPtr<ID3D11Device> &device);
void InitShaders(ComPtr<ID3D11Device> &device);

} // namespace Graphics

} // namespace jRenderer