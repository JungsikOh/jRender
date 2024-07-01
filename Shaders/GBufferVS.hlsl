#include "Common.hlsli"

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float dummy;
};

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

    float4 pos = mul(float4(input.posModel, 1.0), world);
    output.position = mul(pos, viewProj);
    output.texcoord = input.texcoord;
    
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    output.tangentWorld = mul(float4(input.tangentModel, 0.0), world);
    output.tangentWorld = normalize(output.tangentWorld);

    return output;
}