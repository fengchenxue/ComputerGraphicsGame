#include"ShaderStruct.hlsli"
TextureCube skyTexture:register(t3);
SamplerState samLinear : register(s1);

float4 mainPS(PS_INPUT_SKY input) : SV_TARGET0
{
    return skyTexture.Sample(samLinear, normalize(input.TexCoords));
}

