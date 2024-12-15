#include"ShaderStruct.hlsli"
cbuffer cbVS : register(b0)
{
    float4x4 VP;
};

StructuredBuffer<VS_INSTANCE_GENERAL> InstanceBuffer : register(t0);

PS_INPUT_GBuffer mainVS(VS_INPUT_STATIC input)
{
    VS_INSTANCE_GENERAL instance = InstanceBuffer[input.InstanceID];
    
    PS_INPUT_GBuffer output;
   
    output.worldPosition = mul(input.position, instance.W);
    output.position = mul(output.worldPosition, VP);
    
    output.normal = mul(input.normal, (float3x3) instance.W);
    output.normal = normalize(output.normal);
    
    output.tangent = normalize(mul(input.tangent, (float3x3) instance.W));
    
    output.bitangent = normalize(cross(output.normal, output.tangent));
    
    output.texcoord = input.TexCoords;
    output.MaterialIndex = instance.MaterialIndex;
    
    return output;
}

