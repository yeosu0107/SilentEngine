#include "Light.hlsl"

cbuffer cbSsao : register(b10)
{
	float4x4 gInvProj;
	float4x4 gProjTex;				// 텍스쳐 좌표 카메라 좌표계로 변환
	float4   gOffsetVectors[14];	// 무작위 표본

	float4	 gBlurWeights[3];		// 블러 가중치 

	float2	 gInvRenderTargetSize;	// 렌더 타겟 사이즈

	// 차폐 판정에 쓰이는 수치들
	float    gOcclusionRadius;
	float    gOcclusionFadeStart;
	float    gOcclusionFadeEnd;
	float    gSurfaceEpsilon;
};

cbuffer cbRootConstants : register(b11)
{
	bool gHorizontalBlur;
};

Texture2D gNormalMap    : register(t10);
Texture2D gDepthMap     : register(t11);
Texture2D gRandomVecMap : register(t12);

SamplerState gsamPointClamp : register(s10);
SamplerState gsamLinearClamp : register(s11);
SamplerState gsamDepthMap : register(s12);
SamplerState gsamLinearWrap : register(s13);

static const int gSampleCount = 14;	// 무작위 표본 개수

static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosV : POSITION;
	float2 TexC : TEXCOORD0;
};

VertexOut VS(uint vid : SV_VertexID)
{
	VertexOut vout;

	vout.TexC = gTexCoords[vid];

	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

	float4 ph = mul(vout.PosH, gInvProj);
	vout.PosV = ph.xyz / ph.w;

	return vout;
}


float OcclusionFunction(float distZ)
{
	
	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}
 
float NdcDepthToViewDepth(float z_ndc)
{
	float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
	return viewZ;
}

float4 PS(VertexOut pin) : SV_Target
{
	
	float3 n = normalize(gNormalMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz);
	float pz = gDepthMap.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;
	pz = NdcDepthToViewDepth(pz);


	float3 p = (pz / pin.PosV.z)*pin.PosV;

	float3 randVec = 2.0f*gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f*pin.TexC, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;

	for (int i = 0; i < gSampleCount; ++i)
	{
	
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

		float flip = sign(dot(offset, n));

		float3 q = p + flip * gOcclusionRadius * offset;

		float4 projQ = mul(float4(q, 1.0f), gProjTex);
		projQ /= projQ.w;

		float rz = gDepthMap.SampleLevel(gsamDepthMap, projQ.xy, 0.0f).r;
		rz = NdcDepthToViewDepth(rz);

		float3 r = (rz / q.z) * q;

		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);

		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}

	occlusionSum /= gSampleCount;

	float access = 1.0f - occlusionSum;

	return saturate(pow(access, 6.0f));
}
