//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Cbuffer.hlsl"
#include "Sampler.hlsl"
#include "Texture.hlsl"

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

VS_TEXTURED_OUTPUT InstanceVS(VS_TEXTURED_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0.0f;

	InstanceData instData = gInstanceData[instanceID];
	float4x4 world = instData.mtxGameObject;
	
	output.position = mul(mul(mul(float4(input.position, 1.0f), world), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return output;
}



//////////////////////////////////////////////////////



float4 VSTextureToFullScreen(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ���� �� 
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ������ ��
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ������ �Ʒ�
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ���� ��
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ���� �Ʒ�
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ������ �Ʒ�

	return(float4(0, 0, 0, 0));
};

float4 PSTextureToFullScreen(float4 position : SV_POSITION) : SV_Target
{
	float3 cColor = gScreenTexture[int2(position.xy)].rgb;
	return(float4(cColor, 1.0f));
}