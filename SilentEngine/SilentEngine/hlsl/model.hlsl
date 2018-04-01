//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Light.hlsl"

struct VS_MODEL_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tan : TANGENT;
	float2 uv : TEXCOORD;
	uint4 index : BORNINDEX;
	float3 weight : WEIGHT;
};

VS_TEXTURED_LIGHTING_OUTPUT VSStaticModel(VS_MODEL_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE VSStaticInstanceModel(VS_MODEL_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE output;

	InstanceData instData = gInstanceData[instanceID];
	float4x4 world = instData.mtxGameObject;

	output.mat = instData.nMaterial;
	output.normalW = mul(input.normal, (float3x3)world);
	output.positionW = (float3)mul(float4(input.position, 1.0f), world);
	output.position = mul(mul(mul(float4(input.position, 1.0f), world), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}


VS_TEXTURED_LIGHTING_OUTPUT VSDynamicModel(VS_MODEL_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tanL = float3(0.0f, 0.0f, 0.0f);
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weight.x;
	weights[1] = input.weight.y;
	weights[2] = input.weight.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	for (int i = 0; i < 4; ++i) {
		posL += weights[i] * mul(float4(input.position, 1.0f),
			gBoneTransforms[input.index[i]]).xyz;
		normalL+=weights[i]*mul(input.normal,
			(float3x3)gBoneTransforms[input.index[i]]).xyz;
		tanL+=weights[i]*mul(input.tan,
			(float3x3)gBoneTransforms[input.index[i]]).xyz;
	}

	output.normalW = mul(normalL, (float3x3)gmtxObject);
	output.positionW = (float3)mul(float4(posL, 1.0f), gmtxObject);
	output.position = mul(mul(mul(float4(posL, 1.0f), gmtxObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE VSDynamicInstanceModel(VS_MODEL_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE output;

	InstanceAnimateInfo instData = gDynamicInstanceData[instanceID];
	float4x4 world = instData.gInstmtxObject;

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tanL = float3(0.0f, 0.0f, 0.0f);
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weight.x;
	weights[1] = input.weight.y;
	weights[2] = input.weight.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	for (int i = 0; i < 4; ++i) {
		posL += weights[i] * mul(float4(input.position, 1.0f),
			instData.gInstBoneTransforms[input.index[i]]).xyz;
		normalL += weights[i] * mul(input.normal,
			(float3x3)instData.gInstBoneTransforms[input.index[i]]).xyz;
		tanL += weights[i] * mul(input.tan,
			(float3x3)instData.gInstBoneTransforms[input.index[i]]).xyz;
	}

	output.mat = instData.gInstnMat;
	output.normalW = mul(normalL, (float3x3)world);
	output.positionW = (float3)mul(float4(posL, 1.0f), world);
	output.position = mul(mul(mul(float4(posL, 1.0f), world), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

// nPrimitiveID : 삼각형의 정보 
float4 PSStaticModel(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial);

	return(cColor * cIllumination);
};

float4 PSStaticInstanceModel(VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, input.mat);

	return(cColor * cIllumination);
}

//지금은 고정,동적 모델의 PS가 같음
float4 PSDynamicModel(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, gnMat);

	return(cColor * cIllumination);
};

float4 PSDynamicInstanceModel(VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, input.mat);

	return(cColor * cIllumination);
}