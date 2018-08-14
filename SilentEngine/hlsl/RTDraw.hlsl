#ifndef RT_DRAW_HLSL
#define RT_DRAW_HLSL

#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"

VS_TEXTURED_OUTPUT VSRTTextured(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0.0f;
    const float size = 0.4f;

    if (nVertexID == 0)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위 
        output.position = float4(-1.0f, -1.0f + size, 0.0f, 1.0f);
    }
    if (nVertexID == 1)
    {
        output.uv = (float2(1.0f, 0.0f)); // 스크린 오른쪽 위
        output.position = float4(1.0f, -1.0f + size, 0.0f, 1.0f);

    }
    if (nVertexID == 2)
    {
        output.uv = (float2(1.0f, 1.0f)); // 스크린 오른쪽 아래
        output.position = float4(1.0f, -1.0f, 0.0f, 1.0f);

    }
    if (nVertexID == 3)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위
        output.position = float4(-1.0f, -1.0f + size, 0.0f, 1.0f);

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


float4 PSRTTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
    uint index = floor(uv.x * NUM_RENDERTARGET);
    uv.x = frac(uv.x * NUM_RENDERTARGET);

    if (index == GBUFFER_COLOR)
        finalColor = gBuffer[GBUFFER_COLOR].Sample(gDefaultSamplerState, uv);
    else if (index == GBUFFER_OUTLINENRM)
        finalColor = gBuffer[GBUFFER_OUTLINENRM].Sample(gDefaultSamplerState, uv);
    else if (index == GBUFFER_NRM)
        finalColor = gBuffer[GBUFFER_NRM].Sample(gDefaultSamplerState, uv);
 
    if (index == GBUFFER_DEPTH)
    {
        finalColor = gBuffer[GBUFFER_DEPTH].Sample(gDefaultSamplerState, uv).rrra;
    }

    return finalColor;
}

#endif