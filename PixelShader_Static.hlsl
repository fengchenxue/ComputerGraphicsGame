#include"ShaderStruct.hlsli"

Texture2D tex_S:register(t0);
SamplerState samLinear:register(s0);

float4 mainPS(PS_INPUT_STATIC pIn) : SV_Target0
{   
    float4 colour=tex_S.Sample(samLinear,pIn.TexCoords);
    return float4(colour.rgb, 1.0f);
    
}