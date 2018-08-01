#include "NormalMap.hlsl"

struct VS_EFFECT_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
    uint type : TEX_TYPE;
    uint isTorch : EFFECT_TYPE;
};


struct PS_MULTIRENDERTARGET_MONHPBAR
{
    float4 color : SV_TARGET0;
    float4 outlinenormal : SV_TARGET1;
    float4 normal : SV_TARGET2;
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
    output.type = instEffectData.nType;
    output.isTorch = instEffectData.nIsTorch;

	return output;
}

PS_MULTIRENDERTARGET_MONHPBAR PSEffect(VS_EFFECT_OUTPUT input)
{
    PS_MULTIRENDERTARGET_MONHPBAR output = (PS_MULTIRENDERTARGET_MONHPBAR) 0.0f;

    switch (input.type)
    {
        case 0:
            output.color = gUITextures[0].Sample(gDefaultSamplerState, input.uv);
            break;
        case 1:
            output.color = gUITextures[1].Sample(gDefaultSamplerState, input.uv);
            break;
        case 2:
            output.color = gUITextures[2].Sample(gDefaultSamplerState, input.uv);
            break;
        case 3:
            output.color = gUITextures[3].Sample(gDefaultSamplerState, input.uv);
            break;
        default:
            discard;
            break;
    }

    if (output.color.a < 1.0f)
        discard;
    else
        output.color.a = 1.0f;
    output.outlinenormal = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float4 norm = (input.isTorch > 0) ? float4(0.0f, 0.0f, 0.0f, 1.0f) : float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.normal = PackingNorm(norm);
    return output;
}

////////////////////////////////

struct VS_MONHPBAR_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
    uint index : INSTANT_ID;
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
    output.normal = PackingNorm(float4(0.0f, 0.0f, 0.0f, 1.0f));
    output.outlinenormal = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //if (cColor.a < 1.0f)
   //     discard;

    return output;
}
