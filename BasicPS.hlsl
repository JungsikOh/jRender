#include "Common.hlsli"

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

Texture2D albedoTexture : register(t0);

cbuffer MaterialConstants : register(b0)
{
    float3 albedoFactor;
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    // you can use "uint flag" about complex options.
    int useAlbedoMap;
    int useNormalMap;
    int useAOMap;
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    int useBlinnPhong;
    int useBRDF;
};

float3 blinnPhong(float NdotL, float NdotH, float3 diffuseColor, float3 specColor)
{
    
    float3 diffuse = max(NdotL, 0.0) * diffuseColor; // Light Intensity.
    float3 specular = pow(max(NdotH, 0.0), 16.0) * specColor;
    
    return diffuse + specular;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float3 directLighting = float3(0, 0, 0);
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    
    float3 albedoColor = albedoTexture.SampleLevel(linearWrapSampler, input.texcoord, 1).rgb;
    
    float3 ambient = 0.05 * albedoColor;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type)
        {
            float3 lightVec = normalize(lights[i].position - input.posWorld);
            float3 normalVec = input.normalWorld;
            
            float3 halfWay = normalize(lightVec + pixelToEye);
            float NdotL = max(0.0, dot(normalVec, lightVec));
            float NdotH = max(0.0, dot(normalVec, halfWay));
            
            directLighting += blinnPhong(NdotL, NdotH, albedoColor, float3(1.0, 1.0, 1.0));
            directLighting *= (lights[i].spotPower / length(lightVec)) * (lights[i].lightColor / length(lightVec));
        }
    }
    //directLighting = clamp(directLighting, 0.0, 1.0);
    output.pixelColor = float4(ambient + directLighting, 1.0);
    return output;
}