#include"ShaderStruct.hlsli"
Texture2D tex_D : register(t10);
SamplerState samLinear : register(s0);

float4 mainPS(PS_INPUT_DYNAMIC pIn) : SV_Target0
{
    float4 colour = tex_D.Sample(samLinear, pIn.TexCoords);
    return float4(colour.rgb, 1.0f);
}