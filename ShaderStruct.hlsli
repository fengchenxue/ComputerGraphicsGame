#ifndef COMMON_STRUCTURES_HLSLI
#define COMMON_STRUCTURES_HLSLI
struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
#endif