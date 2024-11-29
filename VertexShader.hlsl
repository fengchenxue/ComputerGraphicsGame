#include"ShaderStruct.hlsli"


VertexOut mainVS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.position = float4(vIn.position, 1.0f);
    vOut.color = vIn.color; 
    return vOut;
}

