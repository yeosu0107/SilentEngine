#ifndef DEFFERED_SHADER_HLSL
#define DEFFERED_SHADER_HLSL

#include "Cbuffer.hlsl"
//#include "NormalMap.hlsl"
#include "Sampler.hlsl"
#include "Texture.hlsl"

/*
cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView;
	matrix		gmtxProjection ;
    matrix      gmtxShadowProjection[NUM_DIRECTION_LIGHTS];
    float3      gvCameraPosition;
};

cbuffer cbMaterial : register(b4)
{
	MATERIAL			gMaterials[MAX_MATERIALS];
};

cbuffer cbLights : register(b5)
{
	LIGHT				gLights[MAX_LIGHTS];
	float4				gcGlobalAmbientLight;
};

*/

struct UNPACK_DATA
{
    float depth;
    float3 color;
    float3 norm;
    //float specular;
    //float specpow;
};

// 데이터 언패킹
UNPACK_DATA UNPACKING_GUBBERS(float2 uv)
{
    UNPACK_DATA unpacked_gbuffers = (UNPACK_DATA) 0.0f;

    unpacked_gbuffers.color = gBuffer[GBUFFER_COLOR][(int2) uv].rgb;
    unpacked_gbuffers.norm = (gBuffer[GBUFFER_NRM][(int2) uv].rgb - float3(-0.5f, -0.5f, -0.5f)) * 2.0f;
    unpacked_gbuffers.depth = gBuffer[GBUFFER_DEPTH][(int2) uv].r;

    return unpacked_gbuffers;
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

float4 PS_DEFFERED_SHADER(float4 input : SV_Position) : SV_Target
{
    UNPACK_DATA unpack = UNPACKING_GUBBERS(input.xy);
    LIGHT light = gLights[0];
    MATERIAL mat = gMaterials[0];

    float4 finalColor = (float4) 0.0f;

    finalColor.rgba = float4(unpack.color, 1.0f);

    return finalColor;
    //return finalColor;
};
#endif