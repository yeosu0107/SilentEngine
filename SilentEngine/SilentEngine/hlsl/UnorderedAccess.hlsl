#ifndef UNORDEREDACCESS_HLSL
#define UNORDEREDACCESS_HLSL

RWStructuredBuffer<float> gAverageLum : register(u1);   // ��� �ֵ� 
RWTexture2D<float4> gHDRDownScale : register(u2); // 1/16 Scale ����
RWTexture2D<float4> gBloom : register(u3); // ���� Bloom ���
RWTexture2D<float4> gBloomOutput : register(u4);

#endif