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

const float gamma = 2.2;

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};


float3 GetNormal(PixelShaderInput input)
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
        normalWorld = normalize(mul(normal, TBN));
    
    }
    return normalWorld;
}

float3 blinnPhong(float NdotL, float NdotH, float3 diffuseColor, Light light, float shadow, float intensity)
{
    // diffuseColor = pow(diffuseColor, gamma);
    float3 diffuse = max(NdotL, 0.0) * light.lightColor; // Light Intensity.
    float3 specular = pow(max(NdotH, 0.0), 64.0) * light.lightColor;
    
    float3 color = (diffuse + specular);
    if (light.type & LIGHT_SPOT)
        color *= (1.0 - shadow * intensity) * intensity;
    if (light.type * LIGHT_POINT)
        color *= (1.0 - shadow);
    
    return color * light.spotPower;
}

float CalcDepthInShadow(const in float3 fragPos, float far_plane, float near_plane)
{
    const float c1 = far_plane / (far_plane - near_plane);
    const float c0 = -near_plane * far_plane / (far_plane - near_plane);
    const float3 m = abs(fragPos).xyz;
    const float major = max(m.x, max(m.y, m.z));
    return (c1 * major + c0) / major;
}

float ShadowCalculation(Light light, float3 posWorld, float3 normalWorld, Texture2D shadowMap, TextureCube shadowCubeMap)
{
    float shadow = 1.0;
    if (light.type & LIGHT_SHADOW)
    {
        if (light.type & LIGHT_SPOT)
        {
    // light의 inputTexcoord를 찾아야한다.
    // 빛에서 볼 때를 기준으로 값을 정한다.
    // 1. Light Proj -> Screen(Clip)
            float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
            lightScreen /= lightScreen.w; // Clip -> NDC
            lightScreen.y *= -1; // texcoord는 y축 방향이 다르므로
        
        // 2. [-1, 1] -> [0, 1]
            lightScreen *= 0.5;
            lightScreen += 0.5;
        
            float2 lightTex = float2(lightScreen.x, lightScreen.y);
    
            float2 texel;
            shadowMap.GetDimensions(texel.x, texel.y);
        
            float2 texelSize = 1.0 / texel;
        
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    float closestDepth = shadowMap.Sample(shadowPointSampler, lightTex.xy + float2(x, y) * texelSize).r;
                    float currentDepth = lightScreen.z;
    
                    float bias = 0.005;
                    shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
                }
            }
            shadow /= 9.0;
        }
        else if (light.type & LIGHT_POINT)
        {
            float3 fragToLight = posWorld - light.position;
            //float currentDepth = CalcDepthInShadow(fragToLight, 1.0, 7.5);
            float curDepth = length(fragToLight);
            fragToLight = normalize(fragToLight);
            float closestDepth = shadowCubeMap.Sample(shadowPointSampler, fragToLight).r;
            closestDepth *= 5.0f;
            
            float bias = max(0.005 * (1.0 - dot(normalize(posWorld - light.position), fragToLight)), 0.005);
            shadow = curDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    return shadow;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float3 normalWorld = GetNormal(input);
    
    float3 directLighting = float3(0, 0, 0);
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    
    float3 albedoColor = useAlbedoMap ? albedoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb : albedoFactor;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    { 
        if (lights[i].type)
        {
            float3 ambient = 0.15 * albedoColor;
            
            float3 lightVec = normalize(lights[i].position - input.posWorld);
            float distance = length(lights[i].position - input.posWorld);
            float attenuation = 1 / (distance * distance);
        
            // for Spot Light
            float theta = dot(normalize(-lights[i].direction), lightVec);
            float epsilon = (cos(lights[i].fallOffStart) - cos(lights[i].fallOffEnd));
            
            float3 halfWay = normalize(lightVec + pixelToEye);
            float NdotL = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfWay));
            
            float intensity = clamp((theta - cos(lights[i].fallOffEnd)) / epsilon, 0.0, 1.0);
            
            float shadow = ShadowCalculation(lights[i], input.posWorld, normalWorld, shadowMaps[i], shadowCubeMap);
            
            directLighting += ambient + blinnPhong(NdotL, NdotH, albedoColor, lights[i], shadow, intensity) * albedoColor * attenuation;
        }
    }
    output.pixelColor = float4(directLighting, 1.0);
    return output;
}