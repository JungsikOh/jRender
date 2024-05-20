#include "Common.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D emissiveTex : register(t4);

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

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};


float3 GetNormal(PixelShaderInput input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

float3 blinnPhong(float NdotL, float NdotH, float3 diffuseColor, float3 specColor)
{
    
    float3 diffuse = max(NdotL, 0.0) * diffuseColor; // Light Intensity.
    float3 specular = pow(max(NdotH, 0.0), 16.0) * specColor;
    
    return diffuse + specular;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float3 normalWorld = GetNormal(input);
    
    float3 directLighting = float3(0, 0, 0);
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    
    float3 albedoColor = albedoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb;
    
    float3 ambient = 0.05 * albedoColor;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type)
        {
            float3 lightVec = normalize(lights[i].position - input.posWorld);
            
            float3 halfWay = normalize(lightVec + pixelToEye);
            float NdotL = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfWay));
            
            directLighting += blinnPhong(NdotL, NdotH, albedoColor, float3(1.0, 1.0, 1.0));
            directLighting *= (lights[i].spotPower / length(lightVec)) * (lights[i].lightColor / length(lightVec));
        }
    }
    // directLighting = clamp(directLighting, 0.0, 1.0);
    output.pixelColor = float4(ambient + directLighting, 1.0);
    return output;
}