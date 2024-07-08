#include "Common.hlsli"
#include "LightUtils.hlsli"

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
};

VSToPS VSmain(VertexShaderInput input)
{
    VSToPS output;
    output.pos = float4(input.posModel, 1.0);
    output.pos = mul(output.pos, world);
    output.texcoord = input.texcoord;
    return output;
}

Texture2D DiffuseRoughnessTex : register(t5);
Texture2D NormalMetallicTex : register(t6);
Texture2D EmissiveTex : register(t7);
Texture2D<float> DepthTex : register(t8);
Texture2D ssaoTex : register(t9);

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
};

uint QuerySpecularTextureLevels()
{
    uint width, height, levels;
    specularIBLTex.GetDimensions(0, width, height, levels);
    return levels;
}

float4 PSmain(VSToPS input) : SV_Target0
{
    float4 albedoRoughness = DiffuseRoughnessTex.Sample(linearWrapSampler, input.texcoord);
    float3 albedo = albedoRoughness.rgb;
    float3 emission = EmissiveTex.Sample(linearWrapSampler, input.texcoord).rgb;
    float ao = 1.0f;
    
    if(SSAO)
        ao *= ssaoTex.Sample(linearWrapSampler, input.texcoord).r;
    
    if(IBL)
    {
        float3 ibl = float3(0.0f, 0.0f, 0.0f);
        float4 normalMetallic = NormalMetallicTex.Sample(aniWrapSampler, input.texcoord);
        float3 viewNormal = normalize(2.0f * normalMetallic.rgb - 1.0f);
        float3 worldNormal = normalize(mul(viewNormal, (float3x3) invView));
        float metallic = normalMetallic.a;
        
        float depth = DepthTex.Sample(linearWrapSampler, input.texcoord);
        float3 viewPos = GetViewSpacePosition(input.texcoord, depth);
        float3 worldPos = mul(float4(viewPos, 1.0f), invView);
        
        float3 V = normalize(eyeWorld - worldPos);
        float roughness = albedoRoughness.a;
        if(depth < 1.0f)
        {
            float cosLo = max(0.0f, dot(worldNormal, V));
            float3 irradiance = irradianceIBLTex.Sample(aniWrapSampler, worldNormal).rgb;
            
            float F0 = float3(0.04f, 0.04f, 0.04f);
            F0 = lerp(F0, albedo.rgb, metallic);
            float3 F = FreselSchlickRoughness(cosLo, F0, roughness);
            float3 kd = 1.0f - F;
            kd *= 1.0f - metallic;
            
            float3 diffuseIBL = kd * albedo.rgb * irradiance;
            uint specularTextureLevels = QuerySpecularTextureLevels();
            float3 Lr = reflect(-V, worldNormal);
            
            const float MAX_RELECTION_LOD = min(4.0, specularTextureLevels);
            float3 specularIrradiance = specularIBLTex.SampleLevel(aniWrapSampler, Lr, roughness * MAX_RELECTION_LOD).rgb;
            float2 specularBRDF = brdfTex.Sample(linearClampSampler, float2(cosLo, roughness)).rg;
            float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
            ibl = (diffuseIBL + specularIBL) * strengthIBL;
        }
        return float4(ibl, 1.0f) * ao + float4(emission, 1.0f);
    }
    return float4(albedo, 1.0f) * ao + float4(emission, 1.0f);
}
