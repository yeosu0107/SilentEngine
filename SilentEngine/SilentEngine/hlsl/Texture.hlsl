#ifndef TEXTURE_HLSL
#define TEXTURE_HLSL

#ifndef NUM_DIRECTION_LIGHTS
    #define NUM_DIRECTION_LIGHTS 2
#endif

#ifndef NUM_MAX_TEXTURE
    #define NUM_MAX_TEXTURE 2
#endif

#ifndef NUM_MAX_UITEXTURE
    #define NUM_MAX_UITEXTURE 4
#endif

#ifndef NUM_RENDERTARGET
    #define NUM_RENDERTARGET 4

    #define GBUFFER_COLOR       0
    #define GBUFFER_OUTLINENRM  1
    #define GBUFFER_NRM         2
    #define GBUFFER_DEPTH       3
#endif

#ifndef NUM_HDRRENDERTARGET
    #define NUM_HDRRENDERTARGET 1

    #define GBUFFER_COLOR       0
#endif




struct VS_TEXTURED_INPUT
{
	float3 position : POSITION; // 
	float2 uv : TEXCOORD;		// 
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

struct InstanceData
{
	matrix		mtxGameObject;
	uint			nMaterial;
};

struct InstanceEffectData
{
	uint				nMaxXCount;
	uint				nMaxYCount;
	uint				nNowXCount;
	uint				nNowYCount;
};

struct InstanceAnimateInfo
{
	matrix		gInstmtxObject;
	matrix		gInstBoneTransforms[96];
	uint			gInstnMat;
    uint           gnInstTrashData1;
    uint           gnInstTrashData2;
    uint           gnInstTrashData3;
};

Texture2DArray gBoxTextured : register(t0);
StructuredBuffer<InstanceData> gInstanceData : register(t1);
Texture2DArray gBoxNormal : register(t2);

Texture2D g2DTexture : register(t3);
Texture2D g2DTextureNormal : register(t4);

StructuredBuffer<InstanceEffectData> gEffectInstanceData : register(t5);
StructuredBuffer<InstanceAnimateInfo> gDynamicInstanceData : register(t6);

Texture2D gShadowMap[NUM_DIRECTION_LIGHTS] : register(t7); // register 9 ~ 9 + NUM_DIRECTION_LIGHTS - 1 
Texture2D gUITextures[NUM_MAX_UITEXTURE] : register(t9);

Texture2D gTextures[NUM_MAX_TEXTURE] : register(t13);
Texture2D gNormalTextures[NUM_MAX_TEXTURE] : register(t15);

Texture2D<float4> gBuffer[NUM_RENDERTARGET] : register(t17);
Texture2D<float4> gHDRBuffer[NUM_HDRRENDERTARGET] : register(t22);

StructuredBuffer<float> AverageValues1D : register(t24);
StructuredBuffer<float> gAverageValues: register(t25);

Texture2D<float4> gHDRDownScaleTexture : register(t26);
Texture2D<float4> gBloomInput : register(t27);

#endif