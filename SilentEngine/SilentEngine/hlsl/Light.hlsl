#include "Cbuffer.hlsl"
#include "Texture.hlsl"
#include "Sampler.hlsl"

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT


float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	float fSpecularFactor = 0.0f;
	if (fDiffuseFactor > 0.0f)
	{
		if (gMaterials[gnMaterial].m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#endif
		}
	}

	return((gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular));
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
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
			if (gMaterials[gnMaterial].m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular)) * fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
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
			if (gMaterials[gnMaterial].m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterials[gnMaterial].m_cSpecular.a);
#endif
			}
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterials[gnMaterial].m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterials[gnMaterial].m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterials[gnMaterial].m_cSpecular)) * fAttenuationFactor * fSpotFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gLights[i].m_bEnable)
		{
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera);
			}
		}
	}
	cColor += (gcGlobalAmbientLight * gMaterials[gnMaterial].m_cAmbient);
	cColor.a = gMaterials[gnMaterial].m_cDiffuse.a;

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
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};


VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
};

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gBoxTextured.Sample(gDefaultSamplerState, uvw);
#ifdef _WITH_VERTEX_LIGHTING
	float4 cIllumination = input.color;
#else
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
#endif
	
	return(cColor * cIllumination);
};