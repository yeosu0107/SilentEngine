//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Cbuffer.hlsl"
#include "Sampler.hlsl"

struct VS_MODEL_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint4 index : BORNINDEX;
	float3 weight : WEIGHT;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2DArray gBoxTextured : register(t0);

VS_TEXTURED_OUTPUT VSModelTextured(VS_MODEL_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

// nPrimitiveID : 삼각형의 정보 
float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, 0);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);

	return(cColor);
};