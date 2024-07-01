#include "../Common.hlsli"

Texture2D RenderTex : register(t5);

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(VSToPS input) : SV_TARGET
{
    float3 renderColor = RenderTex.Sample(linearClampSampler, input.texcoord).rgb;
    float4 finalColor = float4(renderColor, 1.0);
    return finalColor;
}