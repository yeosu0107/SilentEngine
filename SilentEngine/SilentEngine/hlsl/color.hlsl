//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Cbuffer.hlsl"
//#include "NormalMap.hlsl"
#include "Sampler.hlsl"
#include "Texture.hlsl"


VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

// nPrimitiveID : 삼각형의 정보 
float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, 0);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);

	return(cColor);
};

VS_TEXTURED_OUTPUT InstanceVS(VS_TEXTURED_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0.0f;

	InstanceData instData = gInstanceData[instanceID];
	float4x4 world = instData.mtxGameObject;
	
	output.position = mul(mul(mul(float4(input.position, 1.0f), world), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return output;
}



//////////////////////////////////////////////////////


float4 VSDeferredFullScreen(uint nVertexID : SV_VertexID) : SV_Position
{
   
    if (nVertexID == 0) { return float4(-1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 1) { return float4(+1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 2) { return float4(+1.0f, -1.0f, 0.0f, 1.0f); }
    if (nVertexID == 3) { return float4(-1.0f, +1.0f, 0.0f, 1.0f); }
    if (nVertexID == 4) { return float4(+1.0f, -1.0f, 0.0f, 1.0f); }
    if (nVertexID == 5) { return float4(-1.0f, -1.0f, 0.0f, 1.0f); }

    return (float4(0, 0, 0, 0));
};

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };				// 가중치의 값
static int2 gnOffsets[9] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };	// ( 중점을 기준으로 첫번째 픽셀 )

float4 PSDeferredFullScreen(float4 position : SV_Position) : SV_Target
{
    //return float4(gShadowMap[1][int2(position.xy)].rrr, 1.0f);
	//return(gNormalTexture[int2(position.xy)]);

	float fEdgeness = 0.0f;
	float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
	
    float3 cColor = (float3) 0.0f;
    const int2 scale = int2(gBlurScale.x / 2, gBlurScale.y / 2);
    //gBlurScale / 2;

    if (gEnable >= 1.0f)
    {
        for (int j = -scale.x; j < scale.x; j++)
        {
            for (int i = -scale.y; i < scale.y; i++)
            {
                cColor += gScreenTexture[(int2) position.xy + int2(j, i)].xyz;
            }
        }

        cColor = cColor / (float) ((gBlurScale.x + 1) * (gBlurScale.y + 1));
        
        if (scale.x < 2 && scale.y < 2)
            cColor = gScreenTexture[(int2) position.xy].xyz;
    }

    //  안죽으면 외곽선도 처리
    else
    {
        if ((position.x >= 1) || (position.y >= 1) || (position.x <= gNormalTexture.Length.x - 2) || (position.y <= gNormalTexture.Length.y - 2))
        {
            float3 vNormal = float3(1.0f, 1.0f, 1.0f);
            for (int i = 0; i < 9; i++)
            {
                vNormal = gNormalTexture[int2(position.xy) + gnOffsets[i]].xyz;
                vNormal = vNormal * 2.0f - 1.0f;
                cEdgeness += gfLaplacians[i] * vNormal;
            }
            fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.3f + cEdgeness.b * 0.4f;
            cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
        }
	
        cColor = gScreenTexture[(int2) position.xy].xyz;
        cColor = (fEdgeness < 0.25f) ? cColor : ((fEdgeness < 0.55) ? (cColor - cEdgeness) : float3(1.0f, 1.0f, 1.0f) - cEdgeness);
    }

	return(float4(cColor, 1.0f));

}

float4 PSFadeEffect(float4 position : SV_POSITION) : SV_Target
{
    return gFadeColor;
}

float4 PSFullScreen(float4 position : SV_POSITION) : SV_Target
{
    float3 cColor = gScreenTexture[int2(position.xy)].rgb;

    return float4(cColor, 1.0f);
}


float4 PS(float4 position : SV_POSITION) : SV_Target
{
    float3 cColor = gShadowMap[0][int2(position.xy)].rgb;

    return float4(cColor, 1.0f);
}
