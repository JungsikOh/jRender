#include "Common.hlsli"

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

cbuffer InstancedConsts : register(b2)
{
    float3 instanceMat[MAX_INSTANCE];
    int useInstancing;
}

struct VSToGS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
    uint instanceID : SV_InstanceID;
};

VSToGS main(VertexShaderInput input)
{
    VSToGS output;
    if (useInstancing)
        input.posModel += instanceMat[input.instanceID];
    output.posWorld = mul(float4(input.posModel, 1.0), world);
    output.texcoord = input.texcoord;
    return output;
}