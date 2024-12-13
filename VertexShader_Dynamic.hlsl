#include"ShaderStruct.hlsli"
cbuffer cbVS_D : register(b2)
{
    float4x4 VP;
};
StructuredBuffer<VS_INSTANCE_GENERAL> InstanceBuffer : register(t0);
Texture2D<float4> BoneTransformsTexture:register(t2);


float4x4 getBoneTransform(uint BoneID,uint instanceID)
{
    
    return transpose(float4x4(
    BoneTransformsTexture.Load(int3(BoneID * 4, instanceID, 0)),
    BoneTransformsTexture.Load(int3(BoneID * 4 + 1, instanceID, 0)),
    BoneTransformsTexture.Load(int3(BoneID * 4 + 2, instanceID, 0)),
    BoneTransformsTexture.Load(int3(BoneID * 4 + 3, instanceID, 0))
    ));

}

PS_INPUT_GENERAL mainVS(VS_INPUT_DYNAMIC input)
{
    VS_INSTANCE_GENERAL instance = InstanceBuffer[input.InstanceID];
    
    PS_INPUT_GENERAL output;
    float4x4 BoneTransform = getBoneTransform(input.BoneIDs[0], input.InstanceID) * input.BoneWeights[0];
    BoneTransform += getBoneTransform(input.BoneIDs[1], input.InstanceID) * input.BoneWeights[1];
    BoneTransform += getBoneTransform(input.BoneIDs[2], input.InstanceID) * input.BoneWeights[2];
    BoneTransform += getBoneTransform(input.BoneIDs[3], input.InstanceID) * input.BoneWeights[3];

    output.position = mul(input.position, BoneTransform);
    output.position = mul(output.position, instance.W);
    output.position = mul(output.position, VP);
        
    output.Normal = mul(input.normal, (float3x3) BoneTransform);
    output.Normal = mul(output.Normal, (float3x3) instance.W);
    output.Normal = normalize(output.Normal);
        
    output.Tangent = mul(input.tangent, (float3x3) BoneTransform);
    output.Tangent = mul(output.Tangent, (float3x3) instance.W);
    output.Tangent = normalize(output.Tangent);
    
    output.TexCoords = input.TexCoords;
    output.MaterialIndex = instance.MaterialIndex;
    
    return output;
}

