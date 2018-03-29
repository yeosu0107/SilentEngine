Texture2DArray gBoxTextured : register(t0);
Texture2DArray gBoxNormal : register(t2);

Texture2D g2DTexture : register(t3);
Texture2D g2DTextureNormal : register(t4);

struct InstanceData
{
	matrix		mtxGameObject;
	uint			nMaterial;
};

StructuredBuffer<InstanceData> gInstanceData : register(t1);

struct InstanceEffectData
{
	uint				nMaxXCount;
	uint				nMaxYCount;
	uint				nNowXCount;
	uint				nNowYCount;
};

StructuredBuffer<InstanceEffectData> gEffectInstanceData : register(t5);