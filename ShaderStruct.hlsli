#ifndef COMMON_STRUCTURES_HLSLI
#define COMMON_STRUCTURES_HLSLI

struct VS_INSTANCE_GENERAL
{
    float4x4 W;
    int MaterialIndex;
};

struct VS_INPUT_STATIC
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint InstanceID : SV_InstanceID;
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
struct PS_INPUT_GENERAL
{
    float4 position : SV_POSITION;
    float3 Normal: NORMAL;
    float3 Tangent: TANGENT;
    float2 TexCoords: TEXCOORD;
    int MaterialIndex : MATERIALINDEX;
};

struct VS_INPUT_SKY
{
    float4 position : POSITION;
};
struct PS_INPUT_SKY
{
    float4 position : SV_POSITION;
    float3 TexCoords: TEXCOORD;

};

#endif