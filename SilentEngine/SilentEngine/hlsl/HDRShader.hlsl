#ifndef HDR_SHADER_HLSL
#define HDR_SHADER_HLSL

#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"

VS_TEXTURED_OUTPUT VSHDR(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT) 0.0f;
   
    if (nVertexID == 0)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위 
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
    }
    if (nVertexID == 1)
    {
        output.uv = (float2(1.0f, 0.0f)); // 스크린 오른쪽 위
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);

    }
    if (nVertexID == 2)
    {
        output.uv = (float2(1.0f, 1.0f)); // 스크린 오른쪽 아래
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);

    }
    if (nVertexID == 3)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);

    }
    if (nVertexID == 4)
    {
        output.uv = (float2(1.0f, 1.0f)); // 스크린 오른쪽 아래
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);

    }
    if (nVertexID == 5)
    {
        output.uv = (float2(0.0f, 1.0f)); // 스크린 왼쪽 아래
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
    }
   
    return output;
};


float4 PSHDR(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    finalColor = gHDRBuffer[0].Sample(gDefaultSamplerState, uv);
 
    return finalColor;
}
#endif