Texture2DArray gBoxTextured : register(t0);
Texture2DArray gBoxNormal : register(t2);


struct InstanceData
{
	matrix		mtxGameObject;
	uint		nMaterial;
};

StructuredBuffer<InstanceData> gInstanceData : register(t1);