#ifndef UNORDEREDACCESS_HLSL
#define UNORDEREDACCESS_HLSL

RWStructuredBuffer<float> gAverageLum : register(u1);   // 평균 휘도 
RWTexture2D<float4> gHDRDownScale : register(u2); // 1/16 Scale 색상
RWTexture2D<float4> gBloom : register(u3); // 최종 Bloom 결과
RWTexture2D<float4> gBloomOutput : register(u4);

#endif