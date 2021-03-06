#include "Light.hlsl"
#include "FogShader.hlsl"

#ifndef _NORMALMAP_HLSL
#define _NORMALMAP_HLSL
struct VS_NORMAL_INPUT 
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangentU : TANGENT;
};

struct VS_NORMAL_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
};

float4 PackingNorm(float4 nor)
{
    return nor * 0.5f + float4(0.5f, 0.5f, 0.5f, 0.5f);
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float3 reflectPercent = (float3) 0.0f;
    if (length(normal) > 0.1f)
    {    
        float cosIncidentAngle = saturate(dot(normal, lightVec));

        float f0 = 1.0f - cosIncidentAngle;
        float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

        reflectPercent = float3(0.0f, 0.0f, 0.0f);

    }
    else
        reflectPercent = float3(1.0f, 1.0f, 1.0f);
	return reflectPercent;
}



VS_NORMAL_OUTPUT VSNormalMap(VS_NORMAL_INPUT input) {
	VS_NORMAL_OUTPUT output = (VS_NORMAL_OUTPUT)0.0f;

	MATERIAL matData = gMaterials[gnMaterial];

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
	output.tangentW = mul(input.tangentU, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return output;
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSNormalMap(VS_NORMAL_OUTPUT input)
{
	MATERIAL matData = gMaterials[gnMaterial];
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
	
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);
	float  roughness = 0.1f;

	input.normalW = normalize(input.normalW);
	
	float4 normalMapSample = gBoxNormal.Sample(gDefaultSamplerState, float3(input.uv,0.0f));
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, input.normalW, input.tangentW);
	
	const float shininess = (1.0f - roughness) * normalMapSample.a;

	cColor = gBoxTextured.Sample(gDefaultSamplerState, float3(input.uv,0.0f));

	float4 litColor = cColor;

	output.color = litColor;
	output.nrmoutline = float4(input.normalW, 1.0f);
    output.nrm = PackingNorm(float4(bumpedNormalW, shininess));
    output.pos = float4(input.positionW, 1.0f);
	//return cColor;
	return output;
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSModelNormalMap(VS_MODEL_NORMAL_OUTPUT input)
{
    MATERIAL matData = gMaterials[input.mat];
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
	
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);
    float roughness = 0.1f;

    input.normalW = normalize(input.normalW);
	
    float4 normalMapSample = gBoxNormal.Sample(gDefaultSamplerState, float3(input.uv, 0.0f));
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, input.normalW, input.tangentW);
	
    const float shininess = (1.0f - roughness) * normalMapSample.a;

    cColor = gBoxTextured.Sample(gDefaultSamplerState, float3(input.uv, 0.0f));
    float3 toEyeW = normalize(gvCameraPosition - input.positionW);

    float4 litColor = cColor;

    output.color = litColor;
    output.nrmoutline = float4(input.normalW, 1.0f);
    output.nrm = PackingNorm(float4(bumpedNormalW, shininess));
    output.pos = float4(input.positionW, 1.0f);
    return output;
}

#endif