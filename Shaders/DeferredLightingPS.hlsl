#include "Common.hlsli"
#include "LightUtils.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D eTex : register(t4);

Texture2D DiffuseRoughnessTex : register(t5);
Texture2D NormalMetallicTex : register(t6);
Texture2D EmissiveTex : register(t8);
Texture2D<float> DepthTex : register(t9);

cbuffer MaterialConstants : register(b0)
{
    float3 albedoFactor; // baseColor
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float dummy;
};

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 main(VSToPS input) : SV_Target
{
    // Unpack GBuffer
    float depth = DepthTex.Sample(linearWrapSampler, input.texcoord);
    float3 viewPos = GetViewSpacePosition(input.texcoord, depth);
    float3 viewDir = normalize(0.0f.xxx - viewPos);
    
    float4 normalMetallic = NormalMetallicTex.Sample(linearWrapSampler, input.texcoord);
    float3 viewNormal = normalMetallic.rgb * 2.0f - 1.0f;
    float metallic = normalMetallic.a;
    
    float4 diffuseRoughness = DiffuseRoughnessTex.Sample(linearWrapSampler, input.texcoord);
    float roughness = diffuseRoughness.a;
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f); 
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (lights[i].type & LIGHT_POINT)
        {
            Lo += DoPointLightPBR(lights[i], viewPos, viewNormal, viewDir, diffuseRoughness.rgb, metallic, roughness);
        }
        if (lights[i].type & LIGHT_DIRECTIONAL)
        {
            Lo += DoDirectinoalLightPBR(lights[i], viewPos, viewNormal, viewDir, diffuseRoughness.rgb, metallic, roughness);
        }
        if (lights[i].type & LIGHT_SPOT)
        {
            Lo += DoSpotLightPBR(lights[i], viewPos, viewNormal, viewDir, diffuseRoughness.rgb, metallic, roughness);
        }
      
    }
    return float4(Lo, 1.0f);
}