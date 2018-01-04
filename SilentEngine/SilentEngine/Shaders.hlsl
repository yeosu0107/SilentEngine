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


#include "Light.hlsl"

struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * 
 */
//Texture2D gtxTexture : register(t0);
/*
 * Texture2DArray : ��� �ؽ��Ĵ� ũ��� ������ ������ ���̿��� �Ѵ�.
 */
Texture2DArray gtxtTextureArray : register(t0);
SamplerState gSamplerState : register(s0);

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

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
};

// nPrimitiveID : �ﰢ���� ���� 
float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gSamplerState, uvw);

	return(cColor);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define _WITH_VERTEX_LIGHTING

struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;		// ������ �븻 ����
	float2 uv : TEXCOORD;		// UV��ǥ
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	//	nointerpolation float3 normalW : NORMAL;
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
	float4 cColor = gtxtTextureArray.Sample(gSamplerState, uvw);
#ifdef _WITH_VERTEX_LIGHTING
	float4 cIllumination = input.color;
#else
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
#endif
	return(cColor * cIllumination);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 color : SV_TARGET0;
	float4 normal : SV_TARGET1;
};



// ���� ����ϰ� �ؽ��� ���Ǟ� �� �� �ִ� �Լ� 
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gSamplerState, uvw);

#ifdef _WITH_VERTEX_LIGHTING
	output.color = input.color * cColor;
#else
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
	output.color = cIllumination * cColor;
#endif

	// -1 ~ 1�� ���踦 0 ~ 1�� �ٲ� 
	output.normal.xyz = input.normalW.xyz * 0.5f + 0.5f;

	return(output);
};

///////////////////////////////////////////////////////////////////////////////
//
float4 VSTextureToFullScreen(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ���� �� 
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ������ ��
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ������ �Ʒ�
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// ��ũ�� ���� ��
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ���� �Ʒ�
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));	// ��ũ�� ������ �Ʒ�

	return(float4(0, 0, 0, 0));
};

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };				// ����ġ�� ��
static int2 gnOffsets[9] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };	// ( ������ �������� ù��° �ȼ� )

/*	Laplacian �˰���
 *	������	
 *	������				-	8���� �ȼ��� ���ö�þ� ����ġ�� ���ϰ� ���Ѵ�. ���� ���� ���� 0�� ��� ���� ����̴�.
 *	������				
 *						-   �Ѱ���	:	��� ȭ�鿡�� �Ϻ� ��ü���� ������ �ƴٰ� �ȵƴٰ� �ϴµ� �� ������ ���ö�þ� �˰��� ���� �Ѱ��̴�.
 										�ٸ� ������Ʈ�� �ȼ����� �о�� ����� �ϱ� ������ ���� �ִ� �ȼ��� ���� ���Ϳ� ������ �� �� �ִ�. 
										�� ��� ������ ��� ���� �ʴ´�.
 */

//static int2 gnOffsets[9] = { {-1,-1}, {0,-1}, int2(1,-1), int2(-1,0), int2(0,0), int2(1,0), int2(-1,1), int2(0,1), int2(1,1) };

Texture2D<float4> gtxtScene : register(t1);
Texture2D<float4> gtxtNormal : register(t2);

// ������ �������� ��ġ�� �ͼ� position�� ���� �������� ��Ÿ����. 
float4 PSTextureToFullScreenByLaplacianEdge(float4 position : SV_POSITION) : SV_Target
{
	float fEdgeness = 0.0f;
	float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
	if ((position.x >= 1) || (position.y >= 1) || (position.x <= gtxtNormal.Length.x - 2) || (position.y <= gtxtNormal.Length.y - 2))
	{
		for (int i = 0; i < 9; i++)
		{
			float3 vNormal = gtxtNormal[int2(position.xy) + gnOffsets[i]].xyz;
			vNormal = vNormal * 2.0f - 1.0f;
			cEdgeness += gfLaplacians[i] * vNormal;
		}

		// ���� ������ ���� ������ ���� ��ȭ ��Ų��. 
		fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
		cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
	}


	float3 cColor = gtxtScene[int2(position.xy)].rgb;

	// fEdgeness�� ũ�� : ������ ���ɼ��� ���� fEdgeeness�� ������ ���� ����� ���ɼ��� ���� 
	//cColor = (fEdgeness < 0.25f) ? cColor : ((fEdgeness < 0.55f) ? (cColor + cEdgeness) : cEdgeness);

	// ���� ��� : ���� ������ ���� �׵θ� ������ �ٸ���.
	
	return(float4(cColor, 1.0f));
}

//////////////////////////////////////////////

struct VS_UITEXTURED_INPUT
{
	float3 position : POSITION;			// ���� ��ǥ
	float2 uv : TEXCOORD;				// �ؽ��� ��ǥ
	float  texturenumber : TEXTURENUM;	// �ؽ��� ��ȣ
};

struct VS_UITEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float texturenumber : TEXTURENUM;
};

cbuffer cbStaticUIInfo : register(b3)
{
	float	texturenumber : packoffset(c0.x);
	float	texturescale  : packoffset(c0.y);
}

Texture2DArray gUItxTextures : register(t3);
SamplerState gUISamplerState : register(s1);

VS_UITEXTURED_OUTPUT VSUiTextured(VS_UITEXTURED_INPUT input)
{
	VS_UITEXTURED_OUTPUT output;

	output.position = float4(input.position, 1.0f);
	output.uv = input.uv;
	output.texturenumber = input.texturenumber;

	return(output);
};

float4 PSUiTextured(VS_UITEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv , input.texturenumber);
	float4 cColor = gUItxTextures.Sample(gUISamplerState, uvw);

	if (cColor.r == 1.0f && cColor.g == 100.0f/255.0f && cColor.b == 100.0f / 255.0f) discard;
	return(cColor);
}

float3 PSDynamicUiTextured(VS_UITEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv , texturenumber);
	float4 cColor = gUItxTextures.Sample(gUISamplerState, uvw);

	if (cColor.r == 1.0f && cColor.g == 100.0f / 255.0f && cColor.b == 100.0f / 255.0f) discard;
	return(cColor);
}


////////////////////////////////////////////////////////////

struct VS_BILLBOARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;		// UV��ǥ
};

struct VS_BILLBOARD_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbBillBoardInfo : register(b4)
{
	matrix		gmtxBillBoard : packoffset(c0);
	uint		gnBillBoardMaterial : packoffset(c4);
};

Texture2D gBillBoardTextures : register(t5);

VS_BILLBOARD_OUTPUT VSBillBoardDiffused(VS_BILLBOARD_INPUT input)
{
	VS_BILLBOARD_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxBillBoard), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSBillBoardDiffused(VS_BILLBOARD_OUTPUT input) : SV_TARGET
{
	float4 color = gBillBoardTextures.Sample(gSamplerState,input.uv);
	return(color);
}