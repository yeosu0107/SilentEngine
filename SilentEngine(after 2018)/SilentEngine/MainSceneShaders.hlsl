cbuffer CB_MainFade_Info : register(b5)
{
	uint  gTextureNumber : packoffset(c0.x);
	float gTimeElapsed : packoffset(c0.y);
};

struct VS_MAINTEXTURED_INPUT
{
	float3 position : POSITION;			// Á¤Á¡ ÁÂÇ¥
	float2 uv : TEXCOORD;				// ÅØ½ºÃÄ ÁÂÇ¥
};

struct VS_MAINTEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2DArray gMaintxTextures : register(t4);
SamplerState gMainSamplerState : register(s2);

VS_MAINTEXTURED_OUTPUT VSMainTextured(VS_MAINTEXTURED_INPUT input)
{
	VS_MAINTEXTURED_OUTPUT output;

	output.position = float4(input.position, 1.0f);
	output.uv = input.uv;

	return(output);
};

float4 PSMainTextured(VS_MAINTEXTURED_OUTPUT input) : SV_TARGET
{
	float3 uvw = float3(input.uv , gTextureNumber);
	//float4 cColor = gUItxTextures.Sample(gUISamplerState, uvw);
	float4 cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	cColor = cColor * 1.0f;

	return(cColor);
}
