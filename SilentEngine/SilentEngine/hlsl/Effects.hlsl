cbuffer cbFadeEffectInfo : register(b7)
{
	float4 gf4Color;
};

float4 PSFadeeffect(float4 position : SV_POSITION) : SV_TARGET
{
	float3 rgbColor = float3(gf4Color.r, gf4Color.g, gf4Color.b) * gf4Color.a;
	float4 cColor = float4(rgbColor , 1.0f);
	//cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	return(cColor);
}
