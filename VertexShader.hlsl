#include"ShaderStruct.hlsli"
cbuffer staticMeshBuffer: register(b0)
{
    float4x4 W;
    float4x4 VP;

};


PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(input.position,W);
    output.position = mul(output.position,VP);
    output.Normal = mul(input.normal, (float3x3)W);
    output.Tangent = mul(input.tangent, (float3x3)W);
    output.TexCoords = input.TexCoords;
    return output;
}

