#include "Common.hlsli"

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float dummy;
};

cbuffer InstancedConsts : register(b2)
{
    float3 instanceMat[MAX_INSTANCE];
    int useInstancing;
}

struct VSToPS
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 normalWorld : NORMAL0;
    float3 tangentWorld : TANGENT0;
};

VSToPS main(VertexShaderInput input)
{
    VSToPS output;

    if (useInstancing)
    {
        input.posModel += instanceMat[input.instanceID];
    }
    
    float4 pos = mul(float4(input.posModel, 1.0), world);
    output.position = mul(pos, viewProj);
    output.texcoord = input.texcoord;
    
    float3 normal = input.normalModel;
    output.normalWorld = mul(normal, (float3x3) worldIT);
    output.normalWorld = normalize(output.normalWorld);
    
    output.tangentWorld = mul(float4(input.tangentModel, 0.0), world);
    output.tangentWorld = normalize(output.tangentWorld);

    return output;
}