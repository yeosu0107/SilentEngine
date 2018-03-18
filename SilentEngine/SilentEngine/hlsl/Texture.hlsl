Texture2DArray gBoxTextured : register(t0);

struct InstanceData
{
	matrix		mtxGameObject;
	uint		nMaterial;
};

StructuredBuffer<InstanceData> gInstanceData : register(t1);