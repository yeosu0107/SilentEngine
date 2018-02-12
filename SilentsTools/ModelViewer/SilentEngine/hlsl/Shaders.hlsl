#include "Common.hlsl"
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
 * Texture2DArray : 모든 텍스쳐는 크기와 포맷이 동일한 것이여야 한다.
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

// nPrimitiveID : 삼각형의 정보 
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
	float3 normal : NORMAL;		// 정점의 노말 벡터
	float2 uv : TEXCOORD;		// UV좌표
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



// 조명도 사용하고 텍스쳐 매피옫 할 수 있는 함수 
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

	// -1 ~ 1의 관계를 0 ~ 1로 바꿈 
	output.normal.xyz = input.normalW.xyz * 0.5f + 0.5f;

	return(output);
};

///////////////////////////////////////////////////////////////////////////////
//
float4 VSTextureToFullScreen(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위 
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 위
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 위
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 왼쪽 아래
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));	// 스크린 오른쪽 아래

	return(float4(0, 0, 0, 0));
};

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };				// 가중치의 값
static int2 gnOffsets[9] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };	// ( 중점을 기준으로 첫번째 픽셀 )

/*	Laplacian 알고리즘
 *	ㅁㅁㅁ	
 *	ㅁㅁㅁ				-	8개의 픽셀에 라플라시안 가중치를 곱하고 더한다. 만약 더한 값이 0일 경우 같은 평면이다.
 *	ㅁㅁㅁ				
 *						-   한계점	:	결과 화면에서 일부 객체에서 엣지가 됐다가 안됐다가 하는데 그 원인은 라플라시안 알고리즘에 대한 한계이다.
 										다른 오브젝트의 픽셀까지 읽어와 계산을 하기 때문에 옆에 있는 픽셀의 법선 벡터와 동일해 질 수 있다. 
										이 경우 엣지가 출력 되지 않는다.
 */

//static int2 gnOffsets[9] = { {-1,-1}, {0,-1}, int2(1,-1), int2(-1,0), int2(0,0), int2(1,0), int2(-1,1), int2(0,1), int2(1,1) };

Texture2D<float4> gtxtScene : register(t1);
Texture2D<float4> gtxtNormal : register(t2);

// 레스터 라이저를 거치고 와서 position은 실제 포지션을 나타낸다. 
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

		// 엣지 정도에 따라 색상을 조금 변화 시킨다. 
		fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
		cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
	}


	float3 cColor = gtxtScene[int2(position.xy)].rgb;

	// fEdgeness가 크면 : 엣지일 가능성이 높음 fEdgeeness가 작으면 같은 평면일 가능성이 높음 
	//cColor = (fEdgeness < 0.25f) ? cColor : ((fEdgeness < 0.55f) ? (cColor + cEdgeness) : cEdgeness);

	// 최종 결과 : 엣지 강도에 따라 테두리 색상이 다르다.
	
	return(float4(cColor, 1.0f));
}

//////////////////////////////////////////////

struct VS_UITEXTURED_INPUT
{
	float3 position : POSITION;			// 정점 좌표
	float2 uv : TEXCOORD;				// 텍스쳐 좌표
	float  texturenumber : TEXTURENUM;	// 텍스쳐 번호
};

struct VS_UITEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float texturenumber : TEXTURENUM;
};


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


////////////////////////////////////////////////////////////

struct VS_BILLBOARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;		// UV좌표
};

struct VS_BILLBOARD_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
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

struct VS_MODEL_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint4 index : BORNINDEX;
	float3 weight : WEIGHT;
};

VS_TEXTURED_LIGHTING_OUTPUT VSModel(VS_MODEL_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weight.x;
	weights[1] = input.weight.y;
	weights[2] = input.weight.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	for (int i = 0; i < 4; ++i) {
		posL += weights[i] * mul(float4(input.pos, 1.0f), 
			gBoneTransforms[input.index[i]]).xyz;
		normalL += weights[i] * mul(input.normal,
			(float3x3)gBoneTransforms[input.index[i]]);
	}

	//output.positionW = (float3)mul(float4(input.pos, 1.0f), gmtxGameObject);
	output.positionW = (float3)mul(float4(posL, 1.0f), gmtxGameObject);
	
	//output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalW = mul(normalL, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
#ifdef _WITH_VERTEX_LIGHTING
	output.normalW = normalize(output.normalW);
	output.color = Lighting(output.positionW, output.normalW);
#endif
	return(output);
}