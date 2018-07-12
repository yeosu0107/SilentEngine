#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "UnorderedAccess.hlsl"

#ifndef COMPUTESHADERS_HLSL
#define COMPUTESHADERS_HLSL

static const uint MAX_GROUP = 64;
static const uint MAX_THREAD = 1024;
static const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);

// ������ �׷� ����ȭ �޸�
groupshared float SharedPosition[MAX_THREAD];
groupshared float SharedFinal[MAX_GROUP];

float HDRDownScale4x4(uint2 position, uint groupThreadID)
{
    float avgLum = 0.0f;

    if (position.y < Res.y)
    {
        int3 nFullScreenPos = int3(position * 4, 0);
        float4 fDownScales = (float4) 0.0f;

        // ���� ��� ���
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            [unroll]
            for (int j = 0; j < 4; ++j)
            {
                // 16���� �ȼ��� �о��
                fDownScales += gHDRBuffer[0].Load(nFullScreenPos, int2(j, i));
            }
        }
        fDownScales /= 16.0f;

        avgLum = dot(fDownScales, LUM_FACTOR); // �ֵ� ���

        SharedPosition[groupThreadID] = avgLum;
    }

     // ������ �׷� ����ȭ
    GroupMemoryBarrierWithGroupSync();

    return avgLum;
};

// 4���� ��� �ֵ��� ���
float DownScaleLumto4(uint dispatchTreadID, uint groupThreadID, float avgLum)
{
    [unroll]
    for (uint size = 4, step1 = 1, step2 = 2, step3 = 3;
        size < 1024;
        size *= 4, step1 *= 4, step2 *= 4, step3 *= 4)
    {
		// Skip out of bound pixels
        if (groupThreadID % size == 0)
        {
			// Calculate the luminance sum for this step
            float stepAvgLum = avgLum;
            stepAvgLum += dispatchTreadID + step1 < Domain ? SharedPosition[groupThreadID + step1] : avgLum;
            stepAvgLum += dispatchTreadID + step2 < Domain ? SharedPosition[groupThreadID + step2] : avgLum;
            stepAvgLum += dispatchTreadID + step3 < Domain ? SharedPosition[groupThreadID + step3] : avgLum;
		
			// Store the results
            avgLum = stepAvgLum;
            SharedPosition[groupThreadID] = stepAvgLum;
        }

		// Synchronize before next step
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

    // �ȼ� 16ĭ ��� ��� & �ֵ� ���
    avgLum = HDRDownScale4x4(position, groupThreadID.x);
 
    // 1024�� �ֵ��� 4���� �ٿ� ������
    avgLum = DownScaleLumto4(dispatchThreadID.x, groupThreadID.x, avgLum);
    
    // 4���� 1�� �ٿ� ������ & ���ۿ� ����
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

    // 1/4�� �ٿ� ������
    if (dispatchThreadID.x % 4 == 0)
    {
        float addAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 0, avgLum);
        avgLum = addAvgLum;
        SharedFinal[dispatchThreadID.x] = addAvgLum;
    }

    GroupMemoryBarrierWithGroupSync();

    // 1/16���� �ٿ� ������
    if (dispatchThreadID.x % 16 == 0)
    {
        float addAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 1, avgLum);
        avgLum = addAvgLum;
        SharedFinal[dispatchThreadID.x] = addAvgLum;
    }

    GroupMemoryBarrierWithGroupSync();

     // 1/64���� �ٿ� ������
    if (dispatchThreadID.x == 0)
    {
        float addFinalAvgLum = AddAvgLumtoDownScale(dispatchThreadID.x, 4, 2, avgLum);
        addFinalAvgLum /= 64.0f;
        gAverageLum[0] = max(addFinalAvgLum, 0.0001);
    }
};

#endif