#ifndef UI_SHADER_HLSL
#define UI_SHADER_HLSL

#include "UICbuffer.hlsl"
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

float4 PSDefaultUI(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    uv = float2(
    input.uv.x / gnNumSprite.x + float(gnNowSprite.x) / float(gnNumSprite.x),
    input.uv.y / gnNumSprite.y + float(gnNowSprite.y) / float(gnNumSprite.y)
    );

    finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);
    finalColor.a *= gfAlpha;

    return finalColor;
}


float4 PSMiniMap(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    uv = float2(
    input.uv.x / gnNumSprite.x + gfData / gnNumSprite.x, 
    input.uv.y / gnNumSprite.y + gfData2 / gnNumSprite.y
    );

    if (gnTexType == 0) 
        finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);
    else if (gnTexType == 1)
        finalColor = gUITextures[1].Sample(gDefaultSamplerState, uv);
    else if (gnTexType == 2)
        finalColor = gUITextures[2].Sample(gDefaultSamplerState, uv);
    else if (gnTexType == 3)
        finalColor = gUITextures[3].Sample(gDefaultSamplerState, uv);

    finalColor.a *= gfAlpha;

    return finalColor;
}

float4 PSUIHPBar(VS_TEXTURED_OUTPUT input) : SV_Target
{

    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    if (gnTexType == 0) 
        finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);
    else if (gnTexType == 1)
    {
        if(uv.x < gfData)
            finalColor = gUITextures[1].Sample(gDefaultSamplerState, uv);
    }
    else if (gnTexType == 2)
    {
        if (uv.x < gfData)
            finalColor = gUITextures[2].Sample(gDefaultSamplerState, uv);
    }
    finalColor.a *= gfAlpha;

    return finalColor;

}

float4 PSBlockUI(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    uv = float2(input.uv.x / gnNumSprite.x + gfData / gnNumSprite.x, input.uv.y);

    finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);

    finalColor.a *= gfAlpha;

    return finalColor;
}

float4 PSCoolDownUI(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
    float2 newuv = uv;
    float value  = 0.0f;
    const float pi = 3.141592;
    const float f = gfData;

    if (f <= 0.5f)
    {
        float tan = -cos(f * 2.0f * pi) / sin(f * 2.0f * pi);
        uv = float2(input.uv.x / gnNumSprite.x + float(gnNowSprite.x) / float(gnNumSprite.x), input.uv.y);  // 실제 텍스쳐 uv 변환
        newuv = input.uv;
        newuv = newuv * 2.0f - float2(1.0f, 1.0f); // 계산을 위해 -1 ~ 1로 변환
        value = tan * newuv.x; // newuv 좌표는 -1 ~ 1  = - 붙혀줘야 
        if (newuv.x > 0.0f && value > newuv.y)
            return (float4) 0.0f;
    }

    else if( f <= 1.0f)
    {
        float tan = -cos(f * 2.0f * pi) / sin(f * 2.0f * pi);
        uv = float2(input.uv.x / gnNumSprite.x + float(gnNowSprite.x) / float(gnNumSprite.x), input.uv.y); // 실제 텍스쳐 uv 변환
        newuv = input.uv;
        newuv = newuv * 2.0f - float2(1.0f, 1.0f); // 계산을 위해 -1 ~ 1로 변환
        value = tan * newuv.x; // newuv 좌표는 -1 ~ 1  = - 붙혀줘야 

        if (newuv.x <= 0.0f && value < newuv.y)   return (float4) 0.0f;
        else if (newuv.x > 0.0f)                    return (float4) 0.0f;
    }
   
    finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);

    finalColor.a *= gfAlpha;

    return finalColor;
}

float4 PSNumberUI(VS_TEXTURED_OUTPUT input) : SV_Target
{
    //uint gNumberType; // 0 : float, 1 : float 분수 , 2 : float 퍼센트 , 3 : int , 4 : int 분수 , 5 : int 퍼센트
    //uint gNumberMaximumLength; // 문자 최대 길이
    //float gfNumber;
    //float gfNumber2;

    int pos = (int) (input.uv.x * gNumberMaximumLength) / 4;
    int uint4pos = (int) (input.uv.x * gNumberMaximumLength) % 4;
    // 0 -> 4 
    float2 uv = (float2) 0.0f;
    // 0 ~ 5
    float4 finalColor = (float4) 0.0f;

    uv = float2(gfNumber[pos][uint4pos] / 14.0f + frac(input.uv.x * gNumberMaximumLength) / 14.0f, input.uv.y);

    finalColor = gUITextures[0].Sample(gDefaultSamplerState, uv);

    finalColor.a *= gfAlpha;

    return finalColor;
}
#endif