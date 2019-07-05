#ifndef HDR_SHADER_HLSL
#define HDR_SHADER_HLSL

#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"
#include "UnorderedAccess.hlsl"

// 사람 눈이 인식하는 색상 민감도
static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);

float3 ToneMApping(float3 color)
{
    // 픽셀의 Luminance Scale계산
    float lumScale = dot(color, LUM_FACTOR.xyz);
    
    // ToneMapping 공식
    lumScale *= gMiddleGrey / gAverageValues[0];
    lumScale = (lumScale + (lumScale * lumScale) / gLumWhiteSqr) / (1.0f + lumScale);

    return color * lumScale;
}

VS_TEXTURED_OUTPUT VSPostProcessing(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT) 0.0f;
   
    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = (float2(0.0f, 0.0f)); // 스크린 왼쪽 위 
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

float4 PSPostProcessing(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 finalColor = (float4) 0.0f;
    float2 uv = input.uv;
 
    finalColor = gHDRBuffer[0].Sample(gDefaultSamplerState, uv);

    // 톤매핑 적용
    if (gHDREnable >= 1.0f)
        finalColor.xyz = ToneMApping(finalColor.xyz);

    // 블룸 분포 적용
    if (gBloomEnable >= 1.0f)
        finalColor.xyz += gBloomInput.Sample(gDefaultSamplerState, uv).xyz * gBloomScale;

    return float4(finalColor.xyz, 1.0f);
}
#endif
