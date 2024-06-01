#include "Common.hlsli"

Texture2D depthOnlyTex : register(t20);

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float LinearizeDepth(float2 texcoord)
{
    float near = 0.05;
    float far = 50.0;
    
    //float4 posProj;
    
    //posProj.xy = texcoord * 2.0 - 1.0;
    //posProj.y *= -1;
    //posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    //posProj.w = 1.0;

    //// ProjectSpace -> ViewSpace
    //float4 posView = mul(posProj, invProj);
    //posView.xyz /= posView.w;
    
    //return posView.z;
    float depth = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    float z = depth * 2.0 - 1.0; // back to NDC 
    depth = (2.0 * near * far) / (far + near - z * (far - near));
    return depth / far;
}

float4 main(VSToPS input) : SV_Target0
{
    float depth = LinearizeDepth(input.texcoord);
    return float4(float3(depth, depth, depth), 1.0);
}