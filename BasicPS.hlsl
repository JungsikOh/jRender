#include "Common.hlsli"

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

cbuffer MaterialConstants : register(b0)
{
    float3 albedoFactor;
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    // you can use "uint flag" about complex options.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    int useBlinnPhong = 1;
    int useBRDF = 0;
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
    
    float3 ambient = 0.05 * albedoFactor;
    
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
            
            directLighting += blinnPhong(NdotL, NdotH, albedoFactor, float3(0.3, 0.3, 0.3));
            directLighting *= (lights[i].spotPower / length(lightVec)) * (lights[i].lightColor / length(lightVec));

        }
    }
    output.pixelColor = float4(ambient + directLighting, 1.0);
    return output;
}