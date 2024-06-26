#include "Common.hlsli"
cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float2 dummy;
};

cbuffer ShadowLightTransform : register(b2)
{
    matrix shadowViewProj[6];
}

struct VSToGS
{
    float4 posWorld : SV_Position;
    float2 texcoord : TEXCOORD0;
};

struct GSToPS
{
    float4 posProj : SV_POSITION;
    float3 fragPos : POSITION0;
    float2 texcoord : TEXCOORD0;
    uint layer : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(
	triangle VSToGS input[3],
	inout TriangleStream<GSToPS> triStream
)
{
    for (int face = 0; face < 6; ++face)
    {
        GSToPS output;
        output.layer = face;
        for (int i = 0; i < 3; i++)
        {
            output.fragPos = input[i].posWorld;
            output.posProj = mul(input[i].posWorld, shadowViewProj[face]);
            output.texcoord = input[i].texcoord;
            triStream.Append(output);
        }
        triStream.RestartStrip();
    }
}