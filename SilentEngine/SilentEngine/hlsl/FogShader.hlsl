#ifndef _SMOG_SHADER
#define _SMOG_SHADER

#include "Cbuffer.hlsl"

#define LINER_FOG   1.0f
#define EXP_FOG     2.0f
#define EXP2_FOG    3.0f

float4 Fog(float4 cColor, float3 vPosition)
{
    float3 vCameraPos   = gvCameraPosition;
    float3 vPosToCamera = vCameraPos - vPosition;
    float fDistance     = length(vPosToCamera);
    float fFogFactor    = 0.0f;

    if (gFogParameter.x == LINER_FOG)
    {
        float fFogRange = gFogParameter.z - gFogParameter.y;
        fFogFactor = saturate((gFogParameter.z - fDistance) / fFogRange);
    }
    float4 cColorByFog = lerp(gFogColor, cColor, fFogFactor);
    return cColorByFog;
}
#endif 