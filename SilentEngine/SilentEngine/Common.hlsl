cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0);
	uint		gnMaterial : packoffset(c4);
};

cbuffer cbBillBoardInfo : register(b6)
{
	matrix		gmtxBillBoard : packoffset(c0);
	uint		gnBillBoardMaterial : packoffset(c4);
};

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW) 
{
	// �븻�� ������ �о�� �� [ 0, 1 ]�� ������ [ -1 , 1 ]�� ����
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	float3 N = unitNormalW;
	// �׶� ����Ʈ�� ���� ����ȭ 
	float3 T = normalize(tangentW - dot(tangentW, N) * N); 
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// [ -1, 1 ]�� ����� �븻���� ���� ���͸� ���� ��ǥ��� ��ȯ
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}