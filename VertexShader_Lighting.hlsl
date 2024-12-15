struct VS_INPUT
{
	float3 position : POSITION;
};
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 TexCoords : TEXCOORD0;
};
PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = float4(input.position, 1.0f);
    output.TexCoords = float2(
    (input.position.x + 1.0f) * 0.5f, 
    (1.0f - (input.position.y + 1.0f) * 0.5f) );
    return output;
}