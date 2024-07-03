#include "Common.hlsli"

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
    float2 dummy;
};

VSToPS VSmain(VertexShaderInput input)
{
    VSToPS output;
    output.pos = float4(input.posModel, 1.0);
    output.pos = mul(output.pos, world);
    output.texcoord = input.texcoord;
    return output;
}

Texture2D DiffuseTex : register(t5);
Texture2D NormalTex : register(t6);
Texture2D<float> DepthTex : register(t7);
Texture2D NoiseTex : register(t8);

cbuffer kernelSamples : register(b2)
{
    float4 samples[MAX_SAMPLES];
};

float4 PSmain(VSToPS input) : SV_Target0 
{
    float depth = DepthTex.Sample(linearWrapSampler, input.texcoord);
    if(depth < 1.0f) // 깊이 값이 1.0 이상인 경우, skybox이므로 ssao 계산량 최적화를 위해 제외한다.
    {
        float3 position = GetViewSpacePosition(input.texcoord, depth);
        float3 normal = NormalTex.Sample(linearWrapSampler, input.texcoord).xyz;
        normal = normalize(normal * 2.0f - 1.0f);
        
        const float2 noiseScale = float2(1280.0f / 4.0f, 720.0f / 4.0f);
        float3 randomVector = NoiseTex.Sample(linearWrapSampler, input.texcoord * noiseScale).xyz;
        
        float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
        float3 bitangent = normalize(cross(normal, tangent));
        float3x3 TBN = float3x3(tangent, bitangent, normal);
        
        const uint kernelSize = 64;
        const float kernelSizeFloat = 64.0f;
        const float radius = 0.89f;
        const float bias = 0.005f;
        float occlusion = 0.0f;
        
        [unroll]
        for (uint idx = 0; idx < kernelSize; ++idx)
        {
            float3 _sample = mul(samples[idx].xyz, TBN);
            _sample = position + _sample * radius;
            
            float4 offset = float4(_sample, 1.0f);
            offset = mul(offset, proj);
            offset.xyz /= offset.w;
            offset.xy = offset.xy * 0.5f + 0.5f;
            offset.y = (1.0f - offset.y);
            
            float sampleDepth = DepthTex.Sample(linearClampSampler, offset.xy);
            float occluded = (_sample.z + bias <= sampleDepth ? 0.0f : 1.0f);
            float intensity = smoothstep(0.0f, 1.0f, radius / abs(position.z - sampleDepth));
            
            occlusion += occluded * intensity;
        }

        occlusion = 1.0f - (occlusion / kernelSizeFloat);
        occlusion = pow(occlusion, 2.0f);
        
        return float4(occlusion, occlusion, occlusion, 1.0f);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
} 