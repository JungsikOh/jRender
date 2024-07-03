#include "Common.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D emissiveTex : register(t4);

Texture2D DiffuseTex : register(t5);
Texture2D NormalTex : register(t6);
Texture2D<float> DepthTex : register(t7);
Texture2D SSAOTex : register(t8);

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float3 BlinnPhong(float NdotL, float NdotH, float3 diffuseColor, Light light)
{
    // diffuseColor = pow(diffuseColor, gamma);
    float3 diffuse = max(NdotL, 0.0) * light.lightColor; // Light Intensity.
    float3 specular = pow(max(NdotH, 0.0), 16.0) * light.lightColor;
    
    float3 finalColor = diffuse + specular;
    return diffuseColor + finalColor * light.spotPower;
}

float4 main(VSToPS input) : SV_Target
{
    // Unpack GBuffer
    float depth = DepthTex.Sample(linearClampSampler, input.texcoord);
    float3 pos = GetWorldSpacePosition(input.texcoord, depth);
    float3 viewDir = normalize(eyeWorld - pos);
    
    float3 albedo = DiffuseTex.Sample(linearClampSampler, input.texcoord).xyz;
    
    float4 normal = NormalTex.Sample(linearClampSampler, input.texcoord);
    float3 normalWorld = normalize(normal.xyz * 2.0f - 1.0f);
    
    float ambientOcclusion = SSAOTex.Sample(linearClampSampler, input.texcoord).r;
    
    float3 lighting = 0.3f * albedo;
    if (SSAO)
    {
        lighting = 0.3f * albedo * ambientOcclusion;
    }
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type)
        {
            float3 lightDir = normalize(lights[i].position - pos);
            float distance = length(lights[i].position - pos);
            float att = 1.0 / distance;
    
            float NDotL = max(0.0, dot(normalWorld, lightDir));
            float3 halfWay = normalize(viewDir + lightDir);
            float NDotH = max(0.0, dot(normalWorld, halfWay));

            float3 diffuse = max(NDotL, 0.0) * albedo * lights[i].lightColor * att;
            float3 specular = pow(max(NDotH, 0.0), 8.0) * lights[i].lightColor * att;
            lighting += (diffuse + specular) * lights[i].spotPower;
        }
    }
    return float4(lighting, 1.0);
}