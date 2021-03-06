struct InputPixel{
	float4 pos		: SV_POSITION;
	float2 texUV	: TEXCOORD0;
};

SamplerState TextureSampler{
	Filter		= MIN_MAG_MIP_LINEAR;
	AddressU	= Wrap;
	AddressV	= Wrap;
};

Texture2D Texture : register(t0);

float4 P_Shader(InputPixel input) : SV_TARGET {
	float4 color = Texture.Sample(TextureSampler, input.texUV);

	return float4(color.r, color.r, color.r, 1.0f);
}
