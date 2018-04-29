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
    uint texindex : TEXINDEX;
};

VS_TEXTURED_LIGHTING_OUTPUT VSStaticModel(VS_MODEL_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        output.ShadowPosH[i] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[i]);

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
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        output.ShadowPosH[i] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[i]);
    
	output.uv = input.uv;

	return(output);
}

VS_MODEL_NORMAL_OUTPUT VSStaticInstanceNORMModel(VS_MODEL_INPUT input, uint instanceID : SV_InstanceID)
{
    VS_MODEL_NORMAL_OUTPUT output;

    InstanceData instData = gInstanceData[instanceID];
    float4x4 world = instData.mtxGameObject;

    output.mat = instData.nMaterial;
    output.normalW = mul(input.normal, (float3x3) world);
    output.positionW = (float3) mul(float4(input.position, 1.0f), world);
    output.tangentW = mul(input.tan, (float3x3) world);
    output.position = mul(mul(mul(float4(input.position, 1.0f), world), gmtxView), gmtxProjection);
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
        output.ShadowPosH[i] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[i]);
    output.uv = input.uv;

    return (output);
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
    for (int j = 0; j < NUM_DIRECTION_LIGHTS; j++)
        output.ShadowPosH[j] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[j]);
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

    for (int i = 0; i < 4; ++i)
    {
        posL += weights[i] * mul(float4(input.position, 1.0f),
			instData.gInstBoneTransforms[input.index[i]]).xyz;
        normalL += weights[i] * mul(input.normal,
			(float3x3) instData.gInstBoneTransforms[input.index[i]]).xyz;
        tanL += weights[i] * mul(input.tan,
			(float3x3) instData.gInstBoneTransforms[input.index[i]]).xyz;
    }

    output.mat = instData.gInstnMat;
    output.normalW = mul(normalL, (float3x3) world);
    output.positionW = (float3) mul(float4(posL, 1.0f), world);
    output.position = mul(mul(mul(float4(posL, 1.0f), world), gmtxView), gmtxProjection);
    for (int j= 0; j < NUM_DIRECTION_LIGHTS; j++)
        output.ShadowPosH[j] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[j]);
    output.uv = input.uv;

    return (output);
};

// nPrimitiveID : 삼각형의 정보 
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStaticModel(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
    float shadowFactors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float4 factor;
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
    {
        shadowFactors[i] = CalcShadowFactor(input.ShadowPosH[i], i);
        if (shadowFactors[i] <= 0.5f)
            break;
    }
    factor = float4(shadowFactors[0], shadowFactors[1], shadowFactors[2], shadowFactors[3]);

    float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial, factor);

	output.color = cColor * cIllumination;
	output.normal = float4(input.normalW, 1.0f);

	return (output);
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStaticInstanceModel(VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);

    float4 shadowFactor = 1.0f;
    
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        shadowFactor[i] = CalcShadowFactor(input.ShadowPosH[i], i);
    float4 cIllumination = Lighting(input.positionW, input.normalW, input.mat, shadowFactor);

	output.color = cColor * cIllumination;
	output.normal = float4(input.normalW, 1.0f);

	return (output);
};

//지금은 고정,동적 모델의 PS가 같음
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSDynamicModel(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
    float4 shadowFactor = 1.0f;
    
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        shadowFactor[i] = CalcShadowFactor(input.ShadowPosH[i], i);
    float4 cIllumination = Lighting(input.positionW, input.normalW, gnMat, shadowFactor);

	output.color = cColor * cIllumination;
	output.normal = float4(input.normalW, 1.0f);

	return (output);
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSDynamicInstanceModel(VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
    float4 shadowFactor = 1.0f;
    
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        shadowFactor[i] = CalcShadowFactor(input.ShadowPosH[i], i);
    float4 cIllumination = Lighting(input.positionW, input.normalW, input.mat, shadowFactor);

	output.color = cColor * cIllumination;
	output.normal = float4(input.normalW, 1.0f);

	return (output);
}

