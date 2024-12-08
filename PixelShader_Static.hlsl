#include"ShaderStruct.hlsli"

float4 mainPS(PS_INPUT_STATIC pIn) : SV_Target0
{   
    return float4(abs(pIn.Normal)*0.9,1.0);
}