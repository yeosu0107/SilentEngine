
//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0) { 
	matrix gmtxWorld : packoffset(c0); 
};

//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1) { 
	matrix gmtxView : packoffset(c0); 
	matrix gmtxProjection : packoffset(c4); 
};

//���� ���̴��� �Է�
struct VS_INPUT { 
	float3 position : POSITION; 
	float4 color : COLOR; 
};

//���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
struct VS_OUTPUT { 
	float4 position : SV_POSITION; 
	float4 color : COLOR; 
};

struct VS_INSTANCING_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float4x4 mtxTransform : WORLDMATRIX;
	float4 instanceColor : INSTANCECOLOR;
};
struct VS_INSTANCING_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};


//���� ���̴��� �����Ѵ�. 
VS_OUTPUT VSDiffused(VS_INPUT input) {
	VS_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), 
		gmtxProjection);

	output.color = input.color;

	return(output);
}
//�ȼ� ���̴��� �����Ѵ�. 
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET {

	return(input.color); 

}

VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
{
	VS_INSTANCING_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform),
		gmtxView), gmtxProjection);
	output.color = input.color + input.instanceColor;
	return(output);
}
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	return(input.color);
}