#include "Common.hlsli"

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

VSToPS VSmain(VertexShaderInput input)
{
    VSToPS output;
    output.pos = float4(input.posModel, 1.0);
    output.pos = mul(output.pos, world);
    output.texcoord = input.texcoord;
    return output;
}

Texture2D SSAOTex : register(t5);

float4 PSmain(VSToPS input) : SV_Target0
{
    uint width = 1;
    uint height = 1;
    uint mipLevels = 0;
    SSAOTex.GetDimensions(0, width, height, mipLevels);
    
    float2 texelSize = 1.0f / float2(width, height);
    float result = 0.0f;
    
    [unroll]
    for (int x = -2; x < 2; ++x)
    {
        [unroll]
        for (int y = -2; y < 2; ++y)
        {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += SSAOTex.Sample(linearClampSampler, input.texcoord + offset).r;
        }
    }

    return float4(result / (4.0f * 4.0f), result / (4.0f * 4.0f), result / (4.0f * 4.0f), 1.0f);
}