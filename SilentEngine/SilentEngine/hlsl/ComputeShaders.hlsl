#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "UnorderedAccess.hlsl"

#ifndef COMPUTESHADERS_HLSL
#define COMPUTESHADERS_HLSL

#define BLOOM_KERNELHALF 6
#define BLOOM_GROUP_THREAD 128

static const uint MAX_GROUP = 64;
static const uint MAX_THREAD = 1024;
static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);
static const float SAMPLE_WEIGHTS[13] = {
    0.002216,
    0.008764,
    0.026995,
    0.064759,
    0.120985,
    0.176033,
    0.199471,
    0.176033,
    0.120985,
    0.064759,
    0.026995,
    0.008764,
    0.002216
};

// 쓰레드 그룹 동기화 메모리
groupshared float SharedPosition[MAX_THREAD];
groupshared float SharedFinal[MAX_GROUP];

float HDRDownScale4x4(uint2 position, uint groupThreadID)
{
    float avgLum = 0.0f;

    if (position.y < Res.y)
    {
        int3 nFullScreenPos = int3(position * 4, 0);
        float4 fDownScales = (float4) 0.0f;

        // 색상 평균 계산
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            [unroll]
            for (int j = 0; j < 4; ++j)
            {
                // 16개의 픽셀을 읽어옴
                fDownScales += gHDRBuffer[0].Load(nFullScreenPos, int2(j, i));
            }
        }
        fDownScales /= 16.0f;
        //gHDRDownScale[position] = fDownScales;
        avgLum = dot(fDownScales, LUM_FACTOR); // 휘도 계산

        SharedPosition[groupThreadID] = avgLum;
    }

     // 쓰레드 그룹 동기화
    GroupMemoryBarrierWithGroupSync();

    return avgLum;
};

// 4개씩 평균 휘도를 계산
float DownScaleLumto4(uint dispatchTreadID, uint groupThreadID, float avgLum)
{
    [unroll]
    for (uint size = 4, step1 = 1, step2 = 2, step3 = 3;
        size < 1024;
        size *= 4, step1 *= 4, step2 *= 4, step3 *= 4)
    {
        if (groupThreadID % size == 0)
        {
            float stepAvgLum = avgLum;
            stepAvgLum += dispatchTreadID + step1 < Domain ? SharedPosition[groupThreadID + step1] : avgLum;
            stepAvgLum += dispatchTreadID + step2 < Domain ? SharedPosition[groupThreadID + step2] : avgLum;
            stepAvgLum += dispatchTreadID + step3 < Domain ? SharedPosition[groupThreadID + step3] : avgLum;

            avgLum = stepAvgLum;
            SharedPosition[groupThreadID] = stepAvgLum;
        }

        GroupMemoryBarrierWithGroupSync();
    }

    return avgLum;
};

void DownScaleto1(uint dispatchTreadID, uint groupThreadID, uint groupID, float avgLum)
{
    if (groupThreadID == 0)
    {
        float fFinalAvgLum = avgLum;
        uint nOffset = MAX_THREAD / 4;

        fFinalAvgLum += dispatchTreadID + (nOffset * 1) < Domain ?
                SharedPosition[dispatchTreadID + (nOffset * 1)] : avgLum;
        fFinalAvgLum += dispatchTreadID + (nOffset * 2) < Domain ?
                SharedPosition[dispatchTreadID + (nOffset * 2)] : avgLum;
        fFinalAvgLum += dispatchTreadID + (nOffset * 3) < Domain ?
                SharedPosition[dispatchTreadID + (nOffset * 3)] : avgLum;

        fFinalAvgLum /= 1024.0f;

        gAverageLum[groupID] = fFinalAvgLum;
    }
};

[numthreads(1024, 1, 1)]
void DownScaleFirstPass(uint3 groupID: SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
    uint2 position = uint2(dispatchThreadID.x % Res.x, dispatchThreadID.x / Res.x);
    float avgLum = 0.0f;

    // 픽셀 16칸 평균 계산 & 휘도 계산
    avgLum = HDRDownScale4x4(position, groupThreadID.x);
 
    // 1024의 휘도를 4개로 다운 스케일
    avgLum = DownScaleLumto4(dispatchThreadID.x, groupThreadID.x, avgLum);
    
    // 4에서 1로 다운 스케일 & 버퍼에 저장
    DownScaleto1(dispatchThreadID.x, groupThreadID.x, groupID.x, avgLum);
};

float AddAvgLumtoDownScale(uint dispatchThreadID, uint scale, uint offset, float avgLum)
{
    offset = pow(scale, offset);

    float addAvgLum = avgLum;

    for (int i = 1; i < 4; ++i)
    {
        addAvgLum += dispatchThreadID.x + (offset * i) < GroupSize ? SharedFinal[dispatchThreadID.x + (offset * i)] : avgLum;
    }

    return addAvgLum;
}

[numthreads(64, 1, 1)]
void DownScaleSecondPass(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
    float avgLum = 0.0f;
    if (dispatchThreadID.x < GroupSize)
    {
        avgLum = AverageValues1D[(dispatchThreadID.x)];
    }
  
    SharedFinal[dispatchThreadID.x] = avgLum;
    GroupMemoryBarrierWithGroupSync();

    // 1/4로 다운 스케일
    if (dispatchThreadID.x % 4 == 0)
    {
        float addAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 0, avgLum);
        avgLum = addAvgLum;
        SharedFinal[dispatchThreadID.x] = addAvgLum;
    }

    GroupMemoryBarrierWithGroupSync();

    // 1/16으로 다운 스케일
    if (dispatchThreadID.x % 16 == 0)
    {
        float addAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 1, avgLum);
        avgLum = addAvgLum;
        SharedFinal[dispatchThreadID.x] = addAvgLum;
    }

    GroupMemoryBarrierWithGroupSync();

     // 1/64으로 다운 스케일
    if (dispatchThreadID.x == 0)
    {
        float addFinalAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 2, avgLum);
        addFinalAvgLum /= 64.0f;
        gAverageLum[0] = max(addFinalAvgLum, 0.0001);
    }
};

////////////////////////////////// Bloom ///////////////////////////////////////////////

groupshared float4 SharedInput[BLOOM_GROUP_THREAD];
// 블룸 휘도 계산
[numthreads(1024, 1, 1)]
void BloomPass(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 position = uint2(dispatchThreadID.x % Res.x, dispatchThreadID.x / Res.x);
    if (position.y < Res.y)
    {
        float4 color = gHDRDownScaleTexture.Load(int3(position, 0));
        float lum = dot(color, LUM_FACTOR);
        float avgLum = gAverageLum[0];

        float colorScale = saturate(lum - avgLum * BloomThreshold);

        gBloom[position] = color * colorScale;
    }
}
 
// 수직 필터링
[numthreads(BLOOM_GROUP_THREAD, 1, 1)]
void VerticalBloomFilter(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    int2 position = int2(groupID.x, groupIndex - BLOOM_KERNELHALF + (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF * 2) * groupID.x);
    position = clamp(position, int2(0, 0), int2(Res.x - 1, Res.y - 1)); // 텍스쳐 범위 내에서만 읽게 범위 제한

    SharedInput[groupIndex] = gBloomInput.Load(int3(position, 0));

    GroupMemoryBarrierWithGroupSync();

    // 가중치를 이용해 섞기 위해 범위 제한
    if (groupIndex >= BLOOM_KERNELHALF && groupIndex < (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF) &&
        (groupIndex - BLOOM_KERNELHALF + (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF * 2) * groupID.y) < Res.y)
    {
        float4 outColor = (float4) 0.0f;
        [unroll]
        for (int i = -BLOOM_KERNELHALF; i <= BLOOM_KERNELHALF; ++i)
            outColor += SharedInput[groupIndex + i] * SAMPLE_WEIGHTS[i + BLOOM_KERNELHALF];

        gBloomOutput[position] = float4(outColor.rgb, 1.0f);
    }
}

// 수평 필터링
[numthreads(BLOOM_GROUP_THREAD, 1, 1)]
void HorizonBloomFilter(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    int2 position = int2(groupIndex - BLOOM_KERNELHALF + (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF * 2) * groupID.x, groupID.y);
    position = clamp(position, int2(0, 0), int2(Res.x - 1, Res.y - 1));

    // Bloom Input값을 그룹 공용 메모리에 저장
    SharedInput[groupIndex] = gBloomInput.Load(int3(position, 0));

    GroupMemoryBarrierWithGroupSync();

    if (groupIndex >= BLOOM_KERNELHALF && groupIndex < (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF) &&
        (groupIndex - BLOOM_KERNELHALF + (BLOOM_GROUP_THREAD - BLOOM_KERNELHALF * 2) * groupID.x) < Res.x)
    {
        float4 outColor = (float4) 0.0f;
        [unroll]
        for (int i = -BLOOM_KERNELHALF; i <= BLOOM_KERNELHALF; ++i)
            outColor += SharedInput[groupIndex + i] * SAMPLE_WEIGHTS[i + BLOOM_KERNELHALF];

        gBloomOutput[position] = float4(outColor.rgb, 1.0f);
    }
}
#endif