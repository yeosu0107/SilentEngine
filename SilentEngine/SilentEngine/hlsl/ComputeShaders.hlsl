#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "UnorderedAccess.hlsl"

#ifndef COMPUTESHADERS_HLSL
#define COMPUTESHADERS_HLSL

static const uint MAX_GROUP = 64;
static const uint MAX_THREAD = 1024;
static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);

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
                fDownScales += gHDRBuffer[0].Load(nFullScreenPos, int2(j, i));
            }
        }
        fDownScales /= 16.0f;

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
    for (uint i = 4; i < MAX_THREAD; i *= 4)
    {
        if (groupThreadID % i == 0)
        {
            float addAvgLum = avgLum;
            addAvgLum += dispatchTreadID + (1 * i) < Domain ?
                SharedPosition[dispatchTreadID + (1 * i)] : avgLum;
            addAvgLum += dispatchTreadID + (2 * i) < Domain ?
                SharedPosition[dispatchTreadID + (2 * i)] : avgLum;
            addAvgLum += dispatchTreadID + (3 * i) < Domain ?
                SharedPosition[dispatchTreadID + (3 * i)] : avgLum;

            avgLum = addAvgLum;
            SharedPosition[groupThreadID] = addAvgLum;
        }
    }

    GroupMemoryBarrierWithGroupSync();

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

        fFinalAvgLum /= (float) MAX_THREAD;

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

    [unroll]
    for (int i = 1; i < scale; ++i)
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

#endif