#include"ShaderStruct.hlsli"
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
SamplerState samLinear : register(s0);

float4 mainPS(PS_INPUT_GENERAL pIn) : SV_Target0
{
    float4 colour;
    if( pIn.MaterialIndex == 0)
    {
        colour = tex0.Sample(samLinear, pIn.TexCoords);
    }
    else if(pIn.MaterialIndex == 1)
    {
        colour = tex1.Sample(samLinear, pIn.TexCoords);
    }
    else if(pIn.MaterialIndex == 2)
    {
        colour = tex2.Sample(samLinear, pIn.TexCoords);
    }
    else
    {
        colour = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    return float4(colour.rgb, 1.0f);
}