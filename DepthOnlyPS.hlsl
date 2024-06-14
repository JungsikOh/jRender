#include "Common.hlsli"

Texture2D renderTex : register(t20);
Texture2D depthOnlyTex : register(t21);

cbuffer PostEffectsConstants : register(b2)
{
    int mode; // 1: Rendered image, 2: DepthOnly
    int edge;
    float depthScale;
    float fogStrength;
}

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float LinearizeDepth(float2 texcoord)
{
    float near = 0.05;
    float far = 50.0;
    
    float4 posProj;
    
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView.z;
    //float depth = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    //float z = depth * 2.0 - 1.0; // back to NDC 
    //depth = (2.0 * near * far) / (far + near - z * (far - near));
    //return depth / far;
}

float4 main(VSToPS input) : SV_Target0
{
    float3 col = renderTex.Sample(linearClampSampler, input.texcoord).rgb;
    float depth = LinearizeDepth(input.texcoord);
    
    float2 eye = input.texcoord.xy * 2.0 - 1.0;
    eye.y *= -1;
    float eyeLength = length(eye);
    
    // Edge Detection
    const float offset = 1.0 / 300.0;
    
    float2 offsets[9] =
    {
        float2(-offset, offset), // top-left
        float2(0.0f, offset), // top-center
        float2(offset, offset), // top-right
        float2(-offset, 0.0f), // center-left
        float2(0.0f, 0.0f), // center-center
        float2(offset, 0.0f), // center-right
        float2(-offset, -offset), // bottom-left
        float2(0.0f, -offset), // bottom-center
        float2(offset, -offset) // bottom-right    
    };

    float kernel[9] =
    {
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    };
    
    if (edge == 1)
    {
        col = float3(0, 0, 0);
        float3 sampleTex[9];
        for (int i = 0; i < 9; i++)
        {
            sampleTex[i] = renderTex.Sample(linearClampSampler, input.texcoord + offsets[i]).rgb;
        }
    
        for (int i = 0; i < 9; i++)
        {
            col += sampleTex[i] * kernel[i];
        }
    }
    
    if (mode == 1)
    {
        float3 fogColor = float3(1, 1, 1);
        float fogMin = 0.0;
        float fogMax = 8.0;
        
        
        float dist = depth * fogStrength * eyeLength;
        float fogFactor = (fogMax - dist) / (fogMax - fogMin);
        
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        col = lerp(fogColor, col, fogFactor);
    }
    else if (mode == 2)
    {
        col = float3(depth, depth, depth) * depthScale;
    }
    return float4(col, 1.0);
}