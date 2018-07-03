#ifndef DEFFERED_SHADER_HLSL
#define DEFFERED_SHADER_HLSL

#include "Cbuffer.hlsl"
//#include "NormalMap.hlsl"
#include "Sampler.hlsl"
#include "NormalMap.hlsl"
#include "FogShader.hlsl"

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f }; // 가중치의 값
static int2 gnOffsets[9] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } }; // ( 중점을 기준으로 첫번째 픽셀 )

struct VS_OUTPUT
{
    float4 position : SV_Position; // vertex position 
    float2 uvPos : TEXCOORD0;
};

struct UNPACK_DATA
{
    float depth;
    float4 color;
    float4 pos;
    float4 norm;
    //float specular;
    //float specpow;
};

float ConvertZToLinearDepth(float depth)
{
    float linearDepth = gmtxProjection[3][2] / (depth - gmtxProjection[2][2]);
    return linearDepth;
}

// 데이터 언패킹
UNPACK_DATA UNPACKING_GBUFFERS(float2 uv)
{
    int3 uvm = int3(uv, 0);
    UNPACK_DATA unpacked_gbuffers = (UNPACK_DATA) 0.0f;

    unpacked_gbuffers.color = gBuffer[GBUFFER_COLOR].Load(uvm);
    unpacked_gbuffers.norm = normalize(gBuffer[GBUFFER_NRM].Load(uvm) * 2.0 - 1.0);
    float depth = gBuffer[GBUFFER_DEPTH].Load(uvm).r;
    unpacked_gbuffers.depth = ConvertZToLinearDepth(depth);
    unpacked_gbuffers.pos = gBuffer[GBUFFER_POS].Load(uvm);
    return unpacked_gbuffers;
}

float3 ConvertPosition(float2 uv, float depth)
{
    float4 position;
    position.xy = uv.xy * float2(1 / gmtxProjection[0][0], 1 / gmtxProjection[1][1]) * depth;
    position.z = depth;
    position.w = 1.0f;

    return mul(position, gmtxInvProjection).xyz;
}

VS_OUTPUT VS_DEFFERED_SHADER(uint nVertexID : SV_VertexID)
{
    VS_OUTPUT output = (VS_OUTPUT) 0.0f;

    if (nVertexID == 0) 
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);

    if (nVertexID == 1)
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);

    if (nVertexID == 2)
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);

    if (nVertexID == 3)
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);

    if (nVertexID == 4)
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);

    if (nVertexID == 5)
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
    
    output.uvPos = output.position.xy;
    return output;
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

float4 PS_DEFFERED_SHADER(VS_OUTPUT input) : SV_Target
{
    UNPACK_DATA unpack = UNPACKING_GBUFFERS(input.position.xy);
    LIGHT light = gLights[0];
    MATERIAL mat = gMaterials[0];

    float shadowFactor = 0.0f;
    float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);
    float4 finalColor = (float4) 0.0f;
    float3 pos = ConvertPosition(input.uvPos, unpack.depth);

    float3 toEyeW = normalize(gvCameraPosition - unpack.pos.xyz);
    float3 r = reflect(-toEyeW, unpack.norm.xyz);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, unpack.norm.xyz, r);

    shadowFactor = CalcShadowFactor(mul(float4(pos, 1.0f), gmtxShadowProjection[0]), 0);
    finalColor = unpack.color * Lighting(pos, unpack.norm.xyz, 0, 1.0f);
    finalColor.rgb += unpack.norm.w * fresnelFactor * finalColor.rgb;
    finalColor = OutLineAndBlur(int2(input.position.xy), float4(finalColor.xyz, shadowFactor));
    return Fog(finalColor, pos);
    //return finalColor;
};
#endif