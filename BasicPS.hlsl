#include "Common.hlsli"

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

cbuffer MaterialConstants : register(b0)
{
    float3 albedoFactor = float3(1.0, 0, 0);
    float roughnessFactor = 1.0;
    float metallicFactor = 1.0;
    float3 emissionFactor = float3(0.0, 0, 0);

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
    
    float diffuse = NdotL * diffuseColor * 0.5; // Light Intensity.
    float specular = pow(NdotH, 2) * specColor * 0.5;
    
    return diffuse * specular;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float3 directLighting = float3(1, 0, 0);
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type)
        {
            float3 radiance = float3(0, 0, 0);
            float3 lightVec = lights[i].position - input.posWorld;
            lightVec = normalize(lightVec);
            float3 normalVec = input.normalWorld;
            
            float3 halfWay = normalize(lightVec + pixelToEye);
            float NdotL = max(1e-5, dot(normalVec, lightVec));
            float NdotH = max(1e-5, dot(normalVec, halfWay));
            
            directLighting += blinnPhong(NdotL, NdotH, float3(1, 1, 1), float3(0, 0, 0));
        }
    }
    
    output.pixelColor = float4(directLighting, 0.0);
    return output;
}