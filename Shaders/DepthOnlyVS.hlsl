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

float4 main(VertexShaderInput input) : SV_POSITION
{
    if(useInstancing)
        input.posModel += instanceMat[input.instanceID];
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    return mul(pos, viewProj);
}