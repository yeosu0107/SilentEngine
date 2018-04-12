#include "model.hlsl"


struct VS_SHADOW_TEXTURED_LIGHTING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float4 ShadowPosH : POSITION1;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
};

VS_SHADOW_TEXTURED_LIGHTING_OUTPUT VSShadowDynamicModel(VS_MODEL_INPUT input)
{
    VS_SHADOW_TEXTURED_LIGHTING_OUTPUT output;

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
			gBoneTransforms[input.index[i]]).xyz;
        normalL += weights[i] * mul(input.normal,
			(float3x3) gBoneTransforms[input.index[i]]).xyz;
        tanL += weights[i] * mul(input.tan,
			(float3x3) gBoneTransforms[input.index[i]]).xyz;
    }

    output.normalW = mul(normalL, (float3x3) gmtxObject);
    output.positionW = (float3) mul(float4(posL, 1.0f), gmtxObject);
    output.position = mul(mul(mul(float4(posL, 1.0f), gmtxObject), gmtxView), gmtxProjection);
    output.ShadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowProjection);
    output.uv = input.uv;

    return (output);
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSShadowDynamicModel(VS_SHADOW_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

    float3 uvw = float3(input.uv, nPrimitiveID / 2);
    float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);

    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(input.ShadowPosH);

    input.normalW = normalize(input.normalW);
    float4 cIllumination = Lighting(input.positionW, input.normalW, gnMat, shadowFactor);

    output.color = cColor * cIllumination;
    output.normal = float4(input.normalW, 1.0f);

    return (output);
};