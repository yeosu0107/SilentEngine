#include "Common.hlsl"

cbuffer cbFadeEffectInfo : register(b7)
{
	float4 gf4Color : packoffset(c0);
};

float4 VSFadeeffect(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위 
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 위
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 아래
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래

	return(float4(0, 0, 0, 0));
};

float4 PSFadeeffect(float4 position : SV_POSITION) : SV_TARGET
{
	float3 rgbColor = float3(gf4Color.r, gf4Color.g, gf4Color.b) * gf4Color.a;
	float4 cColor = float4(rgbColor , 1.0f);
	//float4 cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	return(cColor);
}
