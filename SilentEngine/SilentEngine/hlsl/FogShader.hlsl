#ifndef _SMOG_SHADER
#define _SMOG_SHADER

#include "Cbuffer.hlsl"

#define LINER_FOG   1.0f
#define EXP_FOG     2.0f
#define EXP2_FOG    3.0f

float4 Fog(float4 cColor, float3 vPosition)
{
    float3 vPosToCamera = gvCameraPosition - vPosition;
    float fDistance     = length(vPosToCamera);
    float fFogFactor    = 0.0f;

    if (gFogParameter.x == LINER_FOG)
    {
        float fFogRange = gFogParameter.z - gFogParameter.y;
        fFogFactor = saturate((gFogParameter.z - fDistance) * 2 / fFogRange);
    }
    float4 cColorByFog = lerp(cColor, gFogColor, fFogFactor);
    return cColorByFog;
}

float4 NormalVectorBehindFog(float3 vNormal, float3 vPosition)
{
    float3 vPosToCamera = gvCameraPosition - vPosition;
    float fDistance = length(vPosToCamera);
    float fRange = gFogParameter.z - gFogParameter.y;
    float factor = saturate((gFogParameter.z - fDistance) / fRange);

    return lerp(float4(0.0f, 0.0f, 0.0f, 1.0f), float4(vNormal, 1.0f), factor);
}
#endif 