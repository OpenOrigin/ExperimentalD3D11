struct InputVertex{
	float4 pos		: SV_POSITION;
	float2 texUV	: TEXCOORD0;
};

struct OutputVertex{
	float4 pos		: SV_POSITION;
	float2 texUV	: TEXCOORD0;
};

OutputVertex V_Shader(InputVertex input){
	OutputVertex output;

	output.pos = input.pos;
	output.texUV = input.texUV;

	return output;
}
