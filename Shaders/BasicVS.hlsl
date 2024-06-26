#include "Common.hlsli"

Texture2D g_heightTexture : register(t0);

cbuffer MeshConstants : register(b0)
{
    matrix world;
    matrix worldIT;
    int useHeightMap;
    float heightScale;
    float dummy;
};

cbuffer InstancedConsts : register(b2)
{
    float3 instanceMat[MAX_INSTANCE];
    int useInstancing;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    //Inverse: 법선 벡터는 표면에 수직이어야 하므로, 변환 행렬이 스케일링을 포함할 경우, 이 스케일링을 취소해야 합니다. 이를 위해 원래 행렬의 역행렬을 사용합니다.
    //Transpose: 회전만을 유지하고 스케일링을 제거하기 위해 역행렬을 전치(transpose)합니다. 이는 선형 대수에서 행렬의 회전 성분만을 추출할 때 사용하는 일반적인 방법입니다.
    // Translation Normal Vector(include Scale, Rotate) -> Don't include Scale Rotate coponents.
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    // Translate world space from tangent Vector
    float4 tangentWorld = float4(input.tangentModel, 0.0f);
    tangentWorld = mul(tangentWorld, world);
    
    if (useInstancing)
    {
        input.posModel += instanceMat[input.instanceID];
    }
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, world);
    
    if (useHeightMap)
    {
        float height = g_heightTexture.SampleLevel(linearClampSampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        pos += float4(output.normalWorld * height * heightScale, 0.0);
    }
    
    output.posWorld = pos.xyz;
    
    pos = mul(pos, viewProj);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.tangentWorld = normalize(tangentWorld.xyz);
    
    return output;
  
}