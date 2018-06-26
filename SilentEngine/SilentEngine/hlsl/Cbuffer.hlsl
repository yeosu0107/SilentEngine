#ifndef _CBUFFER_SHADER
#define _CBUFFER_SHADER


#ifndef NUM_DIRECTION_LIGHTS
    #define NUM_DIRECTION_LIGHTS 1
#endif

#define MAX_LIGHTS			4 
#define MAX_MATERIALS		10 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView;
	matrix		gmtxProjection ;
    matrix      gmtxShadowProjection[NUM_DIRECTION_LIGHTS];
    float3      gvCameraPosition;
};

cbuffer cbObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0);
	uint			gnMaterial : packoffset(c4);
}

cbuffer cbAnimateInfo : register(b3)
{
	matrix		gmtxObject;
	matrix		gBoneTransforms[96];
	uint			gnMat;
}

///////////////////////////////////////////

struct MATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;
};

struct MATERIALDATA
{

};

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 color : SV_TARGET0;
	float4 nrmoutline : SV_TARGET1;
    float4 nrm : SV_TARGET2;
};



struct LIGHT
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular;
	float3				m_vPosition;
	float 				m_fFalloff;
	float3				m_vDirection;
	float 				m_fTheta; //cos(m_fTheta)
	float3				m_vAttenuation;
	float				m_fPhi; //cos(m_fPhi)
	bool				m_bEnable;
	int 				m_nType;
	float				m_fRange;
	float				padding;
};

cbuffer cbMaterial : register(b4)
{
	MATERIAL			gMaterials[MAX_MATERIALS];
};

cbuffer cbLights : register(b5)
{
	LIGHT				gLights[MAX_LIGHTS];
	float4				gcGlobalAmbientLight;
};

cbuffer cbEffect : register(b6)
{
	uint				gMaxXCount;
	uint				gMaxYCount;
	uint				gNowXCount;
	uint				gNowYCount;
}

cbuffer cbFade : register(b7)
{
    float4              gFadeColor;
}

cbuffer cbFog : register(b8)
{
    float4 gFogColor;
    float4 gFogParameter; // float4( FogMode, Start, End, Density); 
}

cbuffer cbUIInfo : register(b9)
{
    float2  gxmf2ScreenPos;
    float2  gxmf2ScreenSize;

    uint2   gnNumSprite;
    uint2   gnNowSprite;

    uint2   gnSize;
    uint    gnTexType;
    float   gfData;

    float2 gfScale;
    float  dump1;
    float  dump2;
};

cbuffer cbSceneBlurInfo : register(b10)
{
    uint2 gBlurScale;
    float gTime;
    float gEnable;
}

static matrix gmtxTexture =
{
    0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f
};

struct VS_MODEL_NORMAL_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float4 ShadowPosH[NUM_DIRECTION_LIGHTS] : POSITION1;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangentW : TANGENT;
    uint mat : MATERIAL;
};

#endif