cbuffer ConstantBuffer : register (b0){
	matrix World;
	matrix ViewProj;
	matrix LightView;
	float3 LightDir;
	float3 CameraDir;
	float3 Mode;
}

struct InputVertex{
	float4 pos		: SV_POSITION;
	float3 normal	: NORMAL;
	float2 texUV	: TEXCOORD0;
	float3 tangent	: TANGENT0;
};

struct OutputVertex{
	float4 pos		: SV_POSITION;
	float3 normal	: NORMAL;
	float2 texUV	: TEXCOORD0;
	float3 tangent	: TANGENT0;
	float3 lightDir : TEXCOORD1;
	float4 lpos		: TEXCOORD2;
};

OutputVertex V_Shader(InputVertex input){
	OutputVertex output;

	// Adjust positions, normals and tangents
	output.pos = mul(input.pos, World);
	output.pos = mul(output.pos, ViewProj);

	output.normal = input.normal;
	output.normal = mul(input.normal, World);
	output.tangent = mul(input.tangent, World);

	output.texUV = input.texUV;

	float4 worldPos = mul(input.pos, World);
	output.lightDir = LightDir.xyz - worldPos.xyz;
	output.lightDir = normalize(output.lightDir);

	//output.lpos = mul(input.pos, mul(World, LightView));
	output.lpos = mul(input.pos, World);
	output.lpos = mul(output.lpos, LightView);
	//output.lpos = mul(output.lpos, Projection);

	return output;
}
