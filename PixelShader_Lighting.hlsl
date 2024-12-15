#include"ShaderStruct.hlsli"
Texture2D<float4> worldPosTexture : register(t5);
Texture2D<float4> normalTexture : register(t6);
Texture2D<float4> albedoTexture : register(t7);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 TexCoords : TEXCOORD0;
};

cbuffer LightBuffer : register(b0)
{
    float3 lightDirection;
    float padding;
    float3 lightColor;
    float lightIntensity;
};
float4 mainPS(PS_INPUT pIn) : SV_TARGET
{
    //float2 texCoord = screenPos.xy;
    //float3 worldPos = worldPosTexture.Sample(samLinear, TexCoords).xyz;
    float3 normal = normalize(normalTexture.Sample(samLinear, pIn.TexCoords).xyz);
    float3 albedo = albedoTexture.Sample(samLinear, pIn.TexCoords).xyz;
    
    float diffuseFactor = saturate(dot(normal, -lightDirection));
    float3 diffuseColor = lightColor * (lightIntensity * diffuseFactor+0.003f);
    float3 finalColor = albedo * diffuseColor;
    
    return float4(finalColor, 1.0f);
}