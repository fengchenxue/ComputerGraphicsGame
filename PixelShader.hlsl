#include"ShaderStruct.hlsli"

cbuffer buffername1 : register(b0)
{
    float2 lights[4];
    float time;
    
};

float dist(float2 screenspace_pos, float2 light_pos)
{
    return length(screenspace_pos - light_pos)/abs(cos(time));
}

float4 mainPS2(VertexOut pIn) : SV_Target0
{
    float3 accumulated = float3(0.0f, 0.0f, 0.0f);
    for(unsigned int i = 0; i < 4; i++)
    {
        float d = dist(pIn.position.xy, lights[i]);
        accumulated+=1.0f/d*50;
    }
    accumulated*= pIn.color.rgb;
    
    return float4(accumulated, 1.0f);
}
float4 mainPS(VertexOut pIn) : SV_Target0
{

   
    return float4(pIn.color.rgb * abs(sin(time)), 1.0f);
}