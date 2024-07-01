#include "Common.hlsli"

Texture2D renderTex : register(t5);
Texture2D depthOnlyTex : register(t6);

cbuffer PostEffectsConstants : register(b2)
{
    int mode; // 1: Rendered image, 2: DepthOnly
    int edge;
    int isEyeAdaptationEnabled;
    float depthScale;
    float gammaScale;
    float fogStrength;
    float exposure;
}

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

// Edge Detection
const float offset = 1.0 / 300.0;
    
static const float2 offsets[9] =
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

static const float kernel[9] =
{
          1, 1, 1,
          1, -8, 1,
          1, 1, 1
};
    


float LinearizeDepth(float2 texcoord)
{
    float near = 0.05;
    float far = 50.0;
    
    float4 posProj;
    
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    //posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
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

float4 TextcoordToView(float2 texcoord)
{
    float4 posProj;
    
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1;
    //posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    posProj.z = depthOnlyTex.Sample(linearClampSampler, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView;
}

float4 main(VSToPS input) : SV_Target0
{
    float3 col = renderTex.Sample(linearClampSampler, input.texcoord).rgb;
    float gammaCollection = 1.0 / gammaScale;
    float3 mapped = float3(1, 1, 1) - exp(-col * exposure);
    col = pow(mapped, gammaCollection);
    float depth = LinearizeDepth(input.texcoord);
    
    if (isEyeAdaptationEnabled)
    {
        float3 luminescence = renderTex.Sample(linearClampSampler, input.texcoord).rgb;
        const float lum = 0.2126f * luminescence.r + 0.7152f * luminescence.g + 0.0722f * luminescence.b;
        const float adjSpeed = 0.05f;
        luminescence = lerp(luminescence, 0.5f / lum * exposure, adjSpeed);
        luminescence = clamp(col, 0.3f, 0.7f);
        col = luminescence;

    }
    
    float3 eyeProj;
    eyeProj.xy = input.texcoord.xy * 2.0 - 1.0;
    eyeProj.y *= 1;
    eyeProj.z = 1.0 / depth;
    float eyeLength = max(length(eyeProj), 0.05);
 
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
        
        
        float dist = length(TextcoordToView(input.texcoord).xyz) * eyeLength;
        float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
        float fogFactor = exp(-distFog * fogStrength);
        
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        col = lerp(fogColor, col, fogFactor);
    }
    else if (mode == 2)
    {
        col = float3(depth, depth, depth) * depthScale;
    }
    
    return float4(col, 1.0);
}