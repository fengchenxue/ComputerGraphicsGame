#ifndef COMMON_STRUCTURES_HLSLI
#define COMMON_STRUCTURES_HLSLI

struct VS_INPUT
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 Normal: NORMAL;
    float3 Tangent: TANGENT;
    float2 TexCoords: TEXCOORD;
};

#endif