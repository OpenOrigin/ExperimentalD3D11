cbuffer ConstantBuffer : register (b0){
	matrix World;
	matrix View;
	matrix Projection;
}

struct InputVertex{
	float4 pos		: SV_POSITION;
	float3 normal	: NORMAL;
	float2 texUV	: TEXCOORD0;
	float3 tangent	: TANGENT0;
};

struct OutputVertex{
	float4 pos : SV_POSITION;
};

OutputVertex V_Shader(InputVertex input){
	OutputVertex output;

	output.pos = mul(input.pos, World);
	output.pos = mul(output.pos, View);
	output.pos = mul(output.pos, Projection);

	return output;
}
