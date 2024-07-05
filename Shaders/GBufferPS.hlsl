#include "Common.hlsli"

Texture2D AlbedoTex : register(t0);
Texture2D NormalTex : register(t1);
Texture2D AOTex : register(t2);
Texture2D MetallicRoughnessTex : register(t3);
Texture2D EmissiveTex : register(t4);

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
    float4 DiffuseRoughness : SV_Target0;
    float4 NormalMetallic   : SV_Target1;
    float4 Emissive         : SV_Target2;
};

float3 GetNormal(VSToPS input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = NormalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb; // 범위 [0, 1]
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
    return normalWorld;
}

PSOutput PackGBuffer(float3 baseColor, float3 viewSpaceNormal, float4 emissive, float roughness, float metallic)
{
    PSOutput output = (PSOutput) 0;
    output.NormalMetallic = float4(0.5f * viewSpaceNormal + 0.5f, metallic);
    output.DiffuseRoughness = float4(baseColor, roughness);
    output.Emissive = float4(emissive.xyz, emissive.w / 256.0f);
    
    return output;
}

PSOutput main(VSToPS input)
{
    float4 albeoColor = useAlbedoMap ? AlbedoTex.Sample(linearWrapSampler, input.texcoord) : float4(albedoFactor, 1.0);
    
    float3 normalWorld = GetNormal(input);
    float3 viewSpaceNormal = normalize(mul(normalWorld, (float3x3) view));
    
    float ao = useAOMap ? AOTex.Sample(linearWrapSampler, input.texcoord).r : 1.0f;
    float metallic = useMetallicMap ? MetallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).b : metallicFactor;
    float roughness = useRoughnessMap ? MetallicRoughnessTex.Sample(linearWrapSampler, input.texcoord).g : roughnessFactor;
    float3 aoRoughnessMetallic = float3(ao, roughness, metallic);
    
    float3 emissiveColor = useEmissiveMap ? EmissiveTex.Sample(linearWrapSampler, input.texcoord).rgb : emissionFactor;
    
    return PackGBuffer(albeoColor.xyz, viewSpaceNormal, float4(emissiveColor, 1.0f), aoRoughnessMetallic.g, aoRoughnessMetallic.b);
}