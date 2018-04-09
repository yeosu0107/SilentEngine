#include "model.hlsl"

//static matrix gmtxTexture = {
//	+0.5f, +0.0f, +0.0f, +0.0f,
//	+0.0f, -0.5f, +0.0f, +0.0f,
//	+0.0f, +0.0f, +1.0f, +0.0f,
//	+0.5f, +0.5f, +0.0f, +1.0f
//};
//
//struct VS_INPUT {
//	float3 position : POSITION;
//	float3 normal	: NORMAL;
//};
//
//struct VS_OUTPUT {
//	float4 position		: SV_POSITION;
//	float4 positionW	: POSITION;
//	float3 normalW		: NORMAL;
//	float4 texCoord		: TEXCOORD0;
//	float3 toProjectorW : TEXCOORD1;
//};
//
//Texture2D gtxtProjection : register(t10);
//Texture2D gShadowMap : register(t11);
//
//float CalcShadowFactor(float4 shadowPosH)
//{
//	shadowPosH.xyz /= shadowPosH.w;
//
//	float depth = shadowPosH.z;
//
//	uint width, height, numMips;
//	gShadowMap.GetDimensions(0, width, height, numMips);
//
//	float dx = 1.0f / (float)width;
//
//	float percentLit = 0.0f;
//	const float2 offsets[9] =
//	{
//		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
//		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
//		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
//	};
//
//	[unroll]
//	for (int i = 0; i < 9; ++i)
//	{
//		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
//			shadowPosH.xy + offsets[i], depth).r;
//	}
//
//	return percentLit / 9.0f;
//}
//
//
//
//VS_OUTPUT VSTextureProjection(VS_INPUT input) {
//	VS_OUTPUT output = (VS_OUTPUT)0.0f;
//
//	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld);
//	output.position = mul(mul(output.positionW, gmtxView), gmtxProjection));
//	output.normalW = normalize(mul(float4(input.normal, 1.0f), gmtxWorld)).xyz;
//	output.texCoord = mul(mul(positionW, mul(gmtxProjectView, gmtxProjectProjection)).gmtxTexture);
//	output.toProjectorW = normalize(gvProjectorPosition.xyz - output.positionW.xyz);
//	
//	return output;
//};
//
//float4 PSTextureProjection(VS_OUTPUT input) : SV_Target{
//	float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial);
//	if (dot(input.toProjectorW) <= 0.0f) return (cIllimination);
//	if (input.texCoord.w <= 0.0) return(cIllumination);
//	return(gtxtProjection.Sample(gssProjection, input.texCoord.xy / input.texCoord.ww) * cIlluminaion);
//
//}

VS_TEXTURED_OUTPUT VSShadowMap(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

void PSShadowMap(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

    float3 uvw = float3(input.uv, nPrimitiveID / 2);
    float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
    input.normalW = normalize(input.normalW);
    float4 cIllumination = Lighting(input.positionW, input.normalW, gnMat);

    output.color = cColor * cIllumination;
    output.normal = float4(input.normalW, 1.0f);

};
