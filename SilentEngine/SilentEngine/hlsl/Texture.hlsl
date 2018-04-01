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
	matrix		gInstBoneTransforms[16];
	uint			gInstnMat;
};



Texture2DArray gBoxTextured : register(t0);
StructuredBuffer<InstanceData> gInstanceData : register(t1);
Texture2DArray gBoxNormal : register(t2);

Texture2D g2DTexture : register(t3);
Texture2D g2DTextureNormal : register(t4);

StructuredBuffer<InstanceEffectData> gEffectInstanceData : register(t5);
Texture2D<float4> gScreenTexture : register(t6);

StructuredBuffer<InstanceAnimateInfo> gDynamicInstanceData : register(t7);


