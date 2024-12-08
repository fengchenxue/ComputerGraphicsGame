#ifndef COMMON_STRUCTURES_HLSLI
#define COMMON_STRUCTURES_HLSLI

struct VS_INSTANCE_STATIC
{
    float4x4 W;
};

struct VS_INPUT_STATIC
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};
struct PS_INPUT_STATIC
{
    float4 position : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

struct VS_INSTANCE_DYNAMIC
{
    float4x4 W;
};

struct VS_INPUT_DYNAMIC{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint4 BoneIDs : BONEIDS;
    float4 BoneWeights : BONEWEIGHTS;
    uint InstanceID : SV_InstanceID;
};

struct PS_INPUT_DYNAMIC{
    float4 position : SV_POSITION;
    float3 Normal: NORMAL;
    float3 Tangent: TANGENT;
    float2 TexCoords: TEXCOORD;
};

#endif