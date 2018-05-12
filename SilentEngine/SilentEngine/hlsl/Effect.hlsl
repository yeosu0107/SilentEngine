#include "NormalMap.hlsl"

struct VS_EFFECT_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
};

VS_EFFECT_OUTPUT VSEffect(VS_TEXTURED_INPUT input, uint instanceID : SV_InstanceID)
{

	InstanceData instData = gInstanceData[instanceID];
	InstanceEffectData instEffectData = gEffectInstanceData[instanceID];

	float4x4 world = instData.mtxGameObject;

    VS_EFFECT_OUTPUT output = (VS_EFFECT_OUTPUT) 0.0f;

	float4 positionW = mul(float4(input.position, 1.0f), world);
	output.positionW = positionW.xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	float x = 1.0f / instEffectData.nMaxXCount;
	float y = 1.0f / instEffectData.nMaxYCount;
	output.uv = float2(input.uv.x * x + x * instEffectData.nNowXCount, input.uv.y * y + y * instEffectData.nNowYCount);
	
	return output;
}

float4 PSEffect(VS_EFFECT_OUTPUT input) : SV_Target
{
    float4 cColor = (float4) 0.0f;

	cColor = g2DTexture.Sample(gDefaultSamplerState, input.uv);
	if(cColor.a < 0.9f)
		cColor.a -= 0.2f;

	clip(cColor.a - 0.1f);

    return Fog(cColor, input.positionW);
}
