#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define MAX_LIGHTS 3
#define MAX_INSTANCE 2
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);

// 공용 텍스쳐들은 t10부터 시작
TextureCube specularIBLTex : register(t10);
TextureCube irradianceIBLTex : register(t11); 
TextureCube envIBLTex : register(t12);
Texture2D brdfTex : register(t13);

Texture2D shadowMaps[MAX_LIGHTS] : register(t15);
TextureCube shadowCubeMap : register(t18);


struct Light
{
    float3 radiance;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
    float3 lightColor;
    float dummy1;
    
    uint type;
    float radius;
    float2 dummy;
    
    matrix viewProj;
    matrix invProj;
};

cbuffer GlobalConstants : register(b1)
{
    matrix view;
    matrix proj; 
    matrix invProj;
    matrix viewProj;
    matrix invViewProj;
    
    float3 eyeWorld;
    float strengthIBL;
    
    int textureToDraw;
    float envLodBias;
    float lodBias;
    float dummy2;
    
    Light lights[MAX_LIGHTS];
};

struct VertexShaderInput
{
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL0; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
    uint instanceID : SV_InstanceID;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
};

static float3 GetViewSpacePosition(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, invProj);
    return homogenousLocation.xyz / homogenousLocation.w;
}

#endif // __COMMON_HLSLI__