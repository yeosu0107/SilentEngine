#include "..\..\hlsl\Common.hlsl"

#define _WITH_VERTEX_LIGHTING

cbuffer cbModelInfo : register(b7)
{
	matrix gBoneTransforms[96];
}

struct VS_MODEL_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint4 index : BORNINDEX;
	float4 weight : WEIGHT;
};

VS_TEXTURED_LIGHTING_OUTPUT VSMain(VS_MODEL_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	float4 posL = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i) {
		posL += input.weight[i] * mul(float4(input.position, 1.0f), gBoneTransforms[input.index[i]]);
	}

	//output.normalW = mul(normalL, (float3x3)gmtxGameObject);
	//output.positionW = (float3)posL;
	//output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}