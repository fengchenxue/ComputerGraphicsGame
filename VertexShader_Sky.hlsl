#include"ShaderStruct.hlsli"
cbuffer cbVS_Fixed : register(b4)
{
    float4x4 VP;
};

PS_INPUT_SKY mainVS(VS_INPUT_SKY input)
{
    PS_INPUT_SKY output;
    output.TexCoords = input.position.xyz;
    float4x4 newVP = VP;
    newVP._41 = 0.0f;
    newVP._42 = 0.0f;
    newVP._43 = 0.0f;
    output.position = mul(input.position, newVP);
    
    return output;
}

