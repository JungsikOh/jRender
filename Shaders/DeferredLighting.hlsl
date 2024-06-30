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
    float3 specular = pow(max(NdotH, 0.0), 64.0) * light.lightColor;
    
    float3 finalColor = diffuse + specular;
    return finalColor * light.spotPower;
}

float4 main(VSToPS input) : SV_Target
{
    // Unpack GBuffer
    float depth = DepthTex.Sample(linearWrapSampler, input.texcoord).r;
    float3 pos = GetViewSpacePosition(input.texcoord, depth);
    float3 baseColorSpec = DiffuseTex.Sample(linearWrapSampler, input.texcoord).xyz;
    float4 normal = NormalTex.Sample(linearWrapSampler, input.texcoord);
    float3 normalWorld = normal.xyz * 2.0 - 1.0;
    
    float3 lightDir = normalize(lights[2].position - pos);
    float3 pixelToEye = normalize(eyeWorld - pos);
    
    float NDotL = dot(normalWorld, lightDir);
    float3 halfWay = normalize(pixelToEye + lightDir);
    float NDotH = dot(normalWorld, halfWay);
    
    return float4(BlinnPhong(NDotL, NDotH, baseColorSpec, lights[2]), 1.0);
}