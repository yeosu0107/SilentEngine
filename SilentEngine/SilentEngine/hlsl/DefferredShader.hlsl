#ifndef DEFFERED_SHADER_HLSL
#define DEFFERED_SHADER_HLSL

#include "Cbuffer.hlsl"
//#include "NormalMap.hlsl"
#include "Sampler.hlsl"
#include "NormalMap.hlsl"
#include "FogShader.hlsl"

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f }; // 가중치의 값
static int2 gnOffsets[9] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } }; // ( 중점을 기준으로 첫번째 픽셀 )


struct UNPACK_DATA
{
    float depth;
    float3 color;
    float4 pos;
    float4 norm;
    //float specular;
    //float specpow;
};

// 데이터 언패킹
UNPACK_DATA UNPACKING_GBUFFERS(float2 uv)
{
    int3 uvm = int3(uv, 0);
    UNPACK_DATA unpacked_gbuffers = (UNPACK_DATA) 0.0f;

    unpacked_gbuffers.color = gBuffer[GBUFFER_COLOR].Load(uvm);
    unpacked_gbuffers.norm = (gBuffer[GBUFFER_NRM].Load(uvm) - float4(-0.5f, -0.5f, -0.5f, -0.5f)) * 2.0f;
    unpacked_gbuffers.depth = gBuffer[GBUFFER_DEPTH].Load(uvm).r;
    unpacked_gbuffers.pos = gBuffer[GBUFFER_POS].Load(uvm);
    return unpacked_gbuffers;
}

float3 ConvertPosition(float2 uv, float depth)
{
    float3 posCS = mul(float4(uv, 0.0f, 1.0f), gmtxInvProjection).xyz;
    posCS.xyz = float3(posCS.xy / posCS.z, 1.0f);

    float fz = gmtxProjection[3][2] / (depth - gmtxProjection[2][2]);
    return posCS * fz;
}

float4 VS_DEFFERED_SHADER(uint nVertexID : SV_VertexID) : SV_Position
{
    if (nVertexID == 0) { return float4(-1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 1) { return float4(+1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 2) { return float4(+1.0f, -1.0f, 0.0f, 1.0f); }
    if (nVertexID == 3) { return float4(-1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 4) { return float4(+1.0f, -1.0f, 0.0f, 1.0f); }
    if (nVertexID == 5) { return float4(-1.0f, -1.0f, 0.0f, 1.0f); }
       
    return (float4) 0.0f;
};

float4 OutLineAndBlur(int2 position, float4 cColor)
{
    float fEdgeness = 0.0f;
    float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
    const int2 scale = int2(gBlurScale.x / 2, gBlurScale.y / 2);
    //gBlurScale / 2;

    if (gEnable >= 1.0f)
    {
        for (int j = -scale.x; j < scale.x; j++)
        {
            for (int i = -scale.y; i < scale.y; i++)
            {
                cColor.xyz += gBuffer[GBUFFER_COLOR][(int2) position.xy + int2(j, i)].xyz;
            }
        }

        cColor.xyz = cColor.xyz / (float) ((gBlurScale.x + 1) * (gBlurScale.y + 1));
        
        if (scale.x < 2 && scale.y < 2)
            cColor.xyz = gBuffer[GBUFFER_COLOR][(int2) position.xy].xyz;
    }

    //  안죽으면 외곽선도 처리
    else
    {
        if ((position.x >= 1) || (position.y >= 1) || (position.x <= gBuffer[GBUFFER_OUTLINENRM].Length.x - 2) || (position.y <= gBuffer[GBUFFER_OUTLINENRM].Length.y - 2))
        {
            float3 vNormal = float3(1.0f, 1.0f, 1.0f);
            for (int i = 0; i < 9; i++)
            {
                vNormal = gBuffer[GBUFFER_OUTLINENRM][int2(position.xy) + gnOffsets[i]].xyz;
                vNormal = vNormal * 2.0f - 1.0f;
                cEdgeness += gfLaplacians[i] * vNormal;
            }
            fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.3f + cEdgeness.b * 0.4f;
            cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
        }
	
        cColor.xyz = cColor.xyz;
        cColor.xyz = (fEdgeness < 0.25f) ? cColor.xyz : ((fEdgeness < 0.55) ? (cColor.xyz - cEdgeness) : float3(1.0f, 1.0f, 1.0f) - cEdgeness);
    }

    return cColor;
}

float4 PS_DEFFERED_SHADER(float4 input : SV_Position) : SV_Target
{
    UNPACK_DATA unpack = UNPACKING_GBUFFERS(input.xy);
    LIGHT light = gLights[0];
    MATERIAL mat = gMaterials[0];
    float shadowFactor = 0.0f;
    float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);
    float4 finalColor = (float4) 0.0f;
    float3 pos = ConvertPosition(input.xy, unpack.depth);

    float3 toEyeW = normalize(gvCameraPosition - unpack.pos.xyz);
    float3 r = reflect(-toEyeW, unpack.norm.xyz);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, unpack.norm.xyz, r);

    shadowFactor = CalcShadowFactor(mul(float4(pos, 1.0f), gmtxShadowProjection[0]), 0);
    finalColor = float4(unpack.color, 1.0f) * Lighting(pos, unpack.norm.xyz, 0, shadowFactor);
    finalColor = OutLineAndBlur(int2(input.xy), float4(finalColor.xyz, 1.0f));
    return finalColor;
    //return finalColor;
};
#endif