#include "Light.hlsl"

static matrix gmtxTexture = {
	+0.5f, +0.0f, +0.0f, +0.0f,
	+0.0f, -0.5f, +0.0f, +0.0f,
	+0.0f, +0.0f, +1.0f, +0.0f,
	+0.5f, +0.5f, +0.0f, +1.0f
};

struct VS_INPUT {
	float3 position : POSITION;
	float3 normal	: NORMAL;
};

struct VS_OUTPUT {
	float4 position		: SV_POSITION;
	float4 positionW	: POSITION;
	float3 normalW		: NORMAL;
	float4 texCoord		: TEXCOORD0;
	float3 toProjectorW : TEXCOORD1;
};

Texture2D gtxtProjection : register(t10);

VS_OUTPUT VSTextureProjection(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0.0f;

	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(output.positionW, gmtxView), gmtxProjection));
	output.normalW = normalize(mul(float4(input.normal, 1.0f), gmtxWorld)).xyz;
	output.texCoord = mul(mul(positionW, mul(gmtxProjectView, gmtxProjectProjection)).gmtxTexture);
	output.toProjectorW = normalize(gvProjectorPosition.xyz - output.positionW.xyz);
	
	return output;
};

float4 PSTextureProjection(VS_OUTPUT input) : SV_Target{
	float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial);
	if (dot(input.toProjectorW) <= 0.0f) return (cIllimination);
	if (input.texCoord.w <= 0.0) return(cIllumination);
	return(gtxtProjection.Sample(gssProjection, input.texCoord.xy / input.texCoord.ww) * cIlluminaion);

}