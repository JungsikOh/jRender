#include "Common.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D emissiveTex : register(t4);

Texture2D DiffuseTex : register(t5);
Texture2D NormalTex : register(t6);
Texture2D<float> DepthTex : register(t7);

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
    float depth = DepthTex.Sample(linearWrapSampler, input.texcoord);
    float3 pos = GetViewSpacePosition(input.texcoord, depth);
    float3 V = normalize(0.0f.xxx - pos);
    
    float3 baseColorSpec = DiffuseTex.Sample(linearWrapSampler, input.texcoord).xyz;
    
    float4 normal = NormalTex.Sample(linearWrapSampler, input.texcoord);
    float3 normalWorld = normal.xyz * 2.0 - 1.0; 
    
    float3 lighting = baseColorSpec * 0.15;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].type)
        {
            float3 lightDir = normalize(lights[i].position - pos);
    
            float NDotL = max(0.0, dot(normalWorld, lightDir));
            float3 halfWay = normalize(V + lightDir);
            float NDotH = max(1e-5, dot(normalWorld, halfWay));

            float3 diffuse = max(NDotL, 0.0) * lights[i].lightColor;
            float3 specular = pow(max(NDotH, 0.0), 16.0) * lights[i].lightColor;
            lighting += (diffuse + specular) * lights[i].spotPower * baseColorSpec;
        }
    }
    return float4(lighting, 1.0);
}