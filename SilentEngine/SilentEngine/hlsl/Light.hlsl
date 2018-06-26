#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT


/*
float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}
*/
float CalcShadowFactor(float4 shadowPosH, int index)
{
    shadowPosH.xyz /= shadowPosH.w;

    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap[index].GetDimensions(0, width, height, numMips);

    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap[index].SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera, uint nMatindex, float fShadowFactor)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	float fSpecularFactor = 0.0f;
	if (fDiffuseFactor > 0.0f)
	{
		if (gMaterials[nMatindex].m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#else
			float3 vHalf = normalize(vToCamera + vToLight);

			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#endif
		}
	}

    return ((gLights[nIndex].m_cAmbient * gMaterials[nMatindex].m_cAmbient)  +
		(gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[nMatindex].m_cDiffuse)  +
		(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[nMatindex].m_cSpecular));
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera, uint nMatindex, float fShadowFactor)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterials[nMatindex].m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterials[nMatindex].m_cAmbient) + 
			(gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[nMatindex].m_cDiffuse) * fShadowFactor +
			(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[nMatindex].m_cSpecular) * fShadowFactor) *
			fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera, uint nMatindex)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	float fDiffuseFactor = 0.0f;
	float fSpecularFactor = 0.0f;
	float fAttenuationFactor = 1.0f;
	float fSpotFactor = 1.0f;

	if (fDistance <= gLights[nIndex].m_fRange)
	{
		
		vToLight /= fDistance;
		fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterials[nMatindex].m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[nMatindex].m_cSpecular.a);
#endif
			}
		}
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);

		fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		//cColor = ceil(cColor * 5) / float(5);
		return(
				(	
					(gLights[nIndex].m_cAmbient * gMaterials[nMatindex].m_cAmbient) +
					(gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[nMatindex].m_cDiffuse) +
					(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[nMatindex].m_cSpecular)
				) * fAttenuationFactor * fSpotFactor
			);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}


float4 Lighting(float3 vPosition, float3 vNormal, uint nMatindex, float4 shadowFactor)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

    bool alreadyDrawshadow = false;
    int j = 0;
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gLights[i].m_bEnable)
		{
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
                cColor += DirectionalLight(i, vNormal, vToCamera, nMatindex, shadowFactor[j]) * shadowFactor[j];
                j++;
            }
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
                cColor += PointLight(i, vPosition, vNormal, vToCamera, nMatindex, shadowFactor[j]);
                j++;
            }
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
                cColor += SpotLight(i, vPosition, vNormal, vToCamera, nMatindex);
            }
		}
	}

	cColor += (gcGlobalAmbientLight * gMaterials[nMatindex].m_cAmbient);
	cColor.a = gMaterials[nMatindex].m_cDiffuse.a;
	
	return(cColor);
}

struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;	// 위치 정보
	float3 normal : NORMAL;		// 정점의 노말 벡터
	float2 uv : TEXCOORD;		// UV좌표
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
    float4 ShadowPosH[NUM_DIRECTION_LIGHTS] : POSITION1;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};



VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	 output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
        output.ShadowPosH[i] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[i]);
	output.uv = input.uv;

	return(output);
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
    float4 shadowFactor = 1.0f;

    for (int i = 0; i < NUM_DIRECTION_LIGHTS; i++)
        shadowFactor[i] = CalcShadowFactor(input.ShadowPosH[i], i);

    float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial, shadowFactor);
	
	output.color = cColor * cIllumination;
	 output.nrmoutline = float4(input.normalW, 1.0f);
    output.nrm = output.nrmoutline;
	return(output);
};

////////////////// Instance //////////////////////

struct VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE
{
	float4	position : SV_POSITION;
	float3	positionW : POSITION;
    float4  ShadowPosH[NUM_DIRECTION_LIGHTS] : POSITION1;
	float3	normalW : NORMAL;
	float2	uv : TEXCOORD;
	uint	mat : MATERIAL;
};


VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE VSInstanceTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE output;

	InstanceData instData = gInstanceData[instanceID];
	float4x4 world = instData.mtxGameObject;

	output.mat = instData.nMaterial;
	 output.normalW = mul(input.normal, (float3x3)world);
	output.positionW = (float3)mul(float4(input.position, 1.0f), world);
	output.position = mul(mul(mul(float4(input.position, 1.0f), world), gmtxView), gmtxProjection);
    for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
        output.ShadowPosH[i] = mul(float4(output.positionW, 1.0f), gmtxShadowProjection[i]);
	output.uv = input.uv;

	return(output);
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSInstanceTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT_INSTANCE input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
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
	output.nrmoutline = float4(input.normalW, 1.0f);
    output.nrm = output.nrmoutline;
	return(output);
};