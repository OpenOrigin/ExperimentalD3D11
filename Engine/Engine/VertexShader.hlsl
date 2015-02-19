// Per frame buffer
cbuffer ConstantBuffer : register (b0){
	matrix World;
	matrix View;
	matrix Projection;
	float3 LightDir;
	float3 CameraDir;
	float3 Mode;
}

// Samplers
SamplerState TextureSampler{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);

struct VS_Out{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texUV  : TEXCOORD0;
	float3 tangent : TANGENT0;
};

struct VS_Out2{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texUV  : TEXCOORD0;
	float3 tangent : TANGENT0;
	float3 lightDir : TEXCOORD1;
};

VS_Out2 V_Shader(VS_Out input){
	VS_Out2 output;

	// Adjust positions, normals and tangents
	output.position = mul(input.position, World);

	output.lightDir = normalize(LightDir - output.position);

	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);

	output.normal = input.normal;
	output.normal = mul(input.normal, World);

	output.tangent = input.tangent;
	output.tangent = mul(input.tangent, World);

	output.texUV = input.texUV;

	return output;
}
