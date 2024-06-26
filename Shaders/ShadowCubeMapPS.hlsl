#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

struct GSToPS
{
    float4 posProj : SV_POSITION;
    float3 fragPos : POSITION0;
    float2 texcoord : TEXCOORD0;
    uint layer : SV_RenderTargetArrayIndex;
};

float4 main(GSToPS input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}