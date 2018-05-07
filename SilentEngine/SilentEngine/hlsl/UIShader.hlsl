#ifndef UI_SHADER_HLSL
#define UI_SHADER_HLSL

#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"

VS_TEXTURED_OUTPUT VSUITextured(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0.0f;

    float2 screenpos = (float2) 0.0f;
    screenpos.x = (gxmf2ScreenPos.x - gxmf2ScreenSize.x / 2.0f) / (gxmf2ScreenSize.x / 2.0f);
    screenpos.y = (gxmf2ScreenPos.y - gxmf2ScreenSize.y / 2.0f) / (gxmf2ScreenSize.y / 2.0f);

    float2 size = (float2) 0.0f;
    size.x = ((float) gnSize.x / gxmf2ScreenSize.x) * gfScale.x;
    size.y = ((float) gnSize.y / gxmf2ScreenSize.y) * gfScale.y;

    if (nVertexID == 0)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위 
        output.position = float4(screenpos.x - size.x, screenpos.y + size.y, 0.0f, 1.0f);
    }
    if (nVertexID == 1)
    {
        output.uv = (float2(1.0f, 0.0f)); // 스크린 오른쪽 위
        output.position = float4(screenpos.x + size.x, screenpos.y + size.y, 0.0f, 1.0f);

    }
    if (nVertexID == 2)
    {
        output.uv = (float2(1.0f, 1.0f)); // 스크린 오른쪽 아래
        output.position = float4(screenpos.x + size.x, screenpos.y - size.y, 0.0f, 1.0f);

    }
    if (nVertexID == 3)
    {
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위
        output.position = float4(screenpos.x - size.x, screenpos.y + size.y, 0.0f, 1.0f);

    }
    if (nVertexID == 4)
    {
        output.uv = (float2(1.0f, 1.0f)); // 스크린 왼쪽 아래
        output.position = float4(screenpos.x + size.x, screenpos.y - size.y, 0.0f, 1.0f);

    }
    if (nVertexID == 5)
    {
        output.uv = (float2(0.0f, 1.0f)); // 스크린 오른쪽 아래
        output.position = float4(screenpos.x - size.x, screenpos.y - size.y, 0.0f, 1.0f);
    }

    return output;
};

float4 PSUIHPBar(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float2 fBackgroundUVpos = input.uv;
    fBackgroundUVpos.x = input.uv.x / 3.0f;
    
    float2 fHPBarUVpos = float2(fBackgroundUVpos.x + 1.0f / 3.0f, fBackgroundUVpos.y);
    float4 finalColor = (float4) 0.0f;

    for (float f = 0.0f; f < 3.0f; f += 1.0f) {
        float2 uv = float2(fBackgroundUVpos.x + f / 3.0f, fBackgroundUVpos.y);
        float4 pixelColor = gUITextures[0].Sample(gDefaultSamplerState, uv);
        
        if(f == 1.0f && uv.y <= 1.0f - gfData)
            continue;

        if(pixelColor.a > 0.9f)
            finalColor = pixelColor;
    }
        
    return finalColor;
}
#endif