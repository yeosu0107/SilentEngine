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

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };				// ����ġ�� ��
static int2 gnOffsets[9] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };	// ( ������ �������� ù��° �ȼ� )

float4 PSTextureToFullScreen(float4 position : SV_POSITION) : SV_Target
{
	//return(gNormalTexture[int2(position.xy)]);
	float fEdgeness = 0.0f;
	float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
	if ((position.x >= 1) || (position.y >= 1) || (position.x <= gNormalTexture.Length.x - 2) || (position.y <= gNormalTexture.Length.y - 2))
	{
		float3 vNormal = float3(1.0f, 1.0f, 1.0f);
		for (int i = 0; i < 9; i++)
		{
			vNormal = gNormalTexture[int2(position.xy) + gnOffsets[i]].xyz;
			vNormal = vNormal * 2.0f - 1.0f;
			cEdgeness += gfLaplacians[i] * vNormal;
		}
	
		// ���� ������ ���� ������ ���� ��ȭ ��Ų��. 
		fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
		cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
	}
	
	float3 cColor = gScreenTexture[int2(position.xy)].rgb;
	
	// fEdgeness�� ũ�� : ������ ���ɼ��� ���� fEdgeeness�� ������ ���� ����� ���ɼ��� ���� 
	cColor = (fEdgeness < 0.25f) ? cColor : ((fEdgeness < 0.55f) ? float4(0.0f, 0.0f, 0.0f, 0.0f) : float4(cEdgeness,1.0f));
	
	// ���� ��� : ���� ������ ���� �׵θ� ������ �ٸ���.
	
	return(float4(cColor, 1.0f));

}