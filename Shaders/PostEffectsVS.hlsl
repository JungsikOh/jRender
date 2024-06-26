#include "Common.hlsli"

struct VSToPS
{
    float4 pos : SV_Position; 
    float4 posModel : POSITION;
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

VSToPS main(VertexShaderInput input)
{
    VSToPS output;
    output.pos = float4(input.posModel, 1.0);
    output.posModel = float4(input.posModel, 1.0);
    output.texcoord = input.texcoord;
    return output;
}