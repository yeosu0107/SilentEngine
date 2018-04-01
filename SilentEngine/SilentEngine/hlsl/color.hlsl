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

// nPrimitiveID : 삼각형의 정보 
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
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위 
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 위
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 아래
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래

	return(float4(0, 0, 0, 0));
};

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };				// 가중치의 값
static int2 gnOffsets[9] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };	// ( 중점을 기준으로 첫번째 픽셀 )

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
	
		// 엣지 정도에 따라 색상을 조금 변화 시킨다. 
		fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
		cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
	}
	
	float3 cColor = gScreenTexture[int2(position.xy)].rgb;
	
	// fEdgeness가 크면 : 엣지일 가능성이 높음 fEdgeeness가 작으면 같은 평면일 가능성이 높음 
	cColor = (fEdgeness < 0.25f) ? cColor : ((fEdgeness < 0.55f) ? float4(0.0f, 0.0f, 0.0f, 0.0f) : float4(cEdgeness,1.0f));
	
	// 최종 결과 : 엣지 강도에 따라 테두리 색상이 다르다.
	
	return(float4(cColor, 1.0f));

}