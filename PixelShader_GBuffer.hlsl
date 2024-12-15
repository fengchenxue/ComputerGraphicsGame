#include"ShaderStruct.hlsli"
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D normal0 : register(t8);
Texture2D normal1 : register(t9);
Texture2D normal2 : register(t10);
SamplerState samLinear : register(s0);



GBufferOutput mainPS(PS_INPUT_GBuffer pIn)
{
    GBufferOutput output;
    output.WorldPosition =pIn.worldPosition;
    //output.Normal = float4(pIn.normal, 1.0f);
    float3 tangentNormal;
    
    if (pIn.MaterialIndex == 0)
    {
        output.Albedo = tex0.Sample(samLinear, pIn.texcoord);
        tangentNormal = normal0.Sample(samLinear, pIn.texcoord);
    }
    else if (pIn.MaterialIndex == 1)
    {
        output.Albedo = tex1.Sample(samLinear, pIn.texcoord);
        tangentNormal = normal1.Sample(samLinear, pIn.texcoord);
    }
    else if (pIn.MaterialIndex == 2)
    {
        output.Albedo = tex2.Sample(samLinear, pIn.texcoord);
        tangentNormal = normal2.Sample(samLinear, pIn.texcoord);
    }
    else
    {
        output.Albedo = float4(1.0f, 0.0f, 0.0f, 1.0f);
        tangentNormal = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    tangentNormal=tangentNormal*2.0f-1.0f;
    float3x3 TBN = float3x3(pIn.tangent, pIn.bitangent, pIn.normal);
    
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    output.Normal = float4(worldNormal, 1.0f);
    
    return output;
}