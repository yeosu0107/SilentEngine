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
	if(cColor.a < 1.0f)
        discard;

    return cColor;
}

////////////////////////////////

struct VS_MONHPBAR_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
    uint index : INSTANT_ID;
};

struct PS_MULTIRENDERTARGET_MONHPBAR
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

VS_MONHPBAR_OUTPUT VSHPBar(VS_TEXTURED_INPUT input, uint instanceID : SV_InstanceID)
{

    InstanceData instData = gInstanceData[instanceID];
    InstanceEffectData instEffectData = gEffectInstanceData[instanceID];

    float4x4 world = instData.mtxGameObject;

    VS_MONHPBAR_OUTPUT output = (VS_MONHPBAR_OUTPUT) 0.0f;

    float4 positionW = mul(float4(input.position, 1.0f), world);
    output.positionW = positionW.xyz;
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.index = instanceID;
    output.uv = input.uv;
	
    return output;
}

PS_MULTIRENDERTARGET_MONHPBAR PSHPBar(VS_MONHPBAR_OUTPUT input)
{
    PS_MULTIRENDERTARGET_MONHPBAR output = (PS_MULTIRENDERTARGET_MONHPBAR) 0.0f;
    float4 data = gHPBarData[input.index];
    
    if (data.y < 1.0f) discard;
    if (input.uv.x > data.x) discard;

    output.color = float4(133.0f, 23.0f, 40.0f, 1.0f) / 255.0f;
    output.color.a = 1.0f;
    output.normal = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //if (cColor.a < 1.0f)
   //     discard;

    return output;
}
