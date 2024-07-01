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
    float dummy3;
};

struct VSToPS
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 normalWorld : NORMAL0;
    float3 tangentWorld : TANGENT0;
};

struct PSOutput
{
    float4 normal : SV_TARGET1;
    float4 diffuse : SV_TARGET0;
};

float3 GetNormal(VSToPS input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb; // 범위 [0, 1]
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]
           
        // OpenGL 용 노멀맵일 경우에는 y 방향을 뒤집어줍니다.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = mul(normal, TBN);
    }
    normalWorld = 0.5 * (normalWorld + 1.0);
    return normalWorld;
}

PSOutput PackGBuffer(float3 baseColor, float3 normal)
{
    PSOutput output = (PSOutput) 0;
    output.normal = float4(normal, 1.0);
    output.diffuse = float4(baseColor, 1.0);
    
    return output;
}

PSOutput main(VSToPS input)
{
    float4 albeoColor = useAlbedoMap ? albedoTex.Sample(linearWrapSampler, input.texcoord) : float4(albedoFactor, 1.0);
    float3 normal = GetNormal(input);
    return PackGBuffer(albeoColor.xyz, normal);
}