#include "NormalMap.hlsl"

VS_NORMAL_OUTPUT VSEffect(VS_NORMAL_INPUT input, uint instanceID : SV_InstanceID) {

	InstanceData instData = gInstanceData[instanceID];
	InstanceEffectData instEffectData = gEffectInstanceData[instanceID];

	float4x4 world = instData.mtxGameObject;

	VS_NORMAL_OUTPUT output = (VS_NORMAL_OUTPUT)0.0f;

	float4 positionW = mul(float4(input.position, 1.0f), world);
	output.positionW = positionW.xyz;
	output.normalW = mul(input.normal, (float3x3)world);
	output.tangentW = mul(input.tangentU, (float3x3)world);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	float x = 1.0f / instEffectData.nMaxXCount;
	float y = 1.0f / instEffectData.nMaxYCount;
	output.uv = float2(input.uv.x * x + x * instEffectData.nNowXCount, input.uv.y * y + y * instEffectData.nNowYCount);
	//output.uv = input.uv;
	return output;
}

float4 PSEffect(VS_NORMAL_OUTPUT input) : SV_Target
{
	//float2 uv = float2(input.x * 0.0f, 0.1f);
	MATERIAL matData = gMaterials[0];

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);
	float  roughness = 0.3f;

	input.normalW = normalize(input.normalW);
	
	cColor = g2DTexture.Sample(gDefaultSamplerState, input.uv);
	if(cColor.a < 0.9f)
		cColor.a -= 0.2f;
	//if (cColor.a - 0.015f < 0.0f)
	//	discard;
	//clip(cColor.a - 0.1f);

	float4 normalMapSample = g2DTextureNormal.Sample(gDefaultSamplerState, input.uv);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, input.normalW, input.tangentW);
	
	const float shininess = (1.0f - roughness) * normalMapSample.a;

	float3 toEyeW = normalize(gvCameraPosition - input.positionW);

	float3 shadowFactor = 1.0f;
    float4 directLight = Lighting(input.positionW, bumpedNormalW, 0.0f, shadowFactor);
	
	float4 litColor = directLight * cColor;
	float3 r = reflect(-toEyeW, bumpedNormalW);
	//float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
	//float3 fresnelFactor = float3(1.0f, 1.0f, 1.0f);
	litColor.rgb += shininess * fresnelFactor * litColor.rgb;

	//return cColor;
	return litColor;
}
