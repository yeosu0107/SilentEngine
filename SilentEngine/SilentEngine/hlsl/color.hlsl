//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Cbuffer.hlsl"
#include "Sampler.hlsl"

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION; // 
	float2 uv : TEXCOORD;		// 
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2DArray gBoxTextured : register(t0);

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

// nPrimitiveID : �ﰢ���� ���� 
float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, 0);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);

	return(cColor);
};