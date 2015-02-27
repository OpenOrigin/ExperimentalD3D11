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
	float3 lightDir : TEXCOORD1;
};

float3 SampleNormalMap(float3 N, float3 T, float2 uv){
	float4 normalMapSample = NormalTexture.Sample(TextureSampler, uv);

	// Adjust normals and tangent
	N = normalize(N);
	normalMapSample = (2.0f * normalMapSample) - 1.0f;
	T = normalize(T - dot(T, N) * N);
	
	// Create tangent space matrix
	float3 biTangent = cross(N, T);
	float3x3 tanSpace = float3x3(T, biTangent, N);
	
	// Run our mapped normal through the tangent space matrix
	N = normalize(mul(normalMapSample, tanSpace));

	return N;
}

float4 BlinnPhong(float3 N, float4 diffuse, float3 lightColor, float3 lightDirection){
	float3 L				= normalize(lightDirection);
	float3 H				= normalize(L + normalize(CameraDir));
	float NdotH				= dot(N, H);
	float3 intensity		= pow(saturate(NdotH), 20);
	float3 specularColor	= intensity * lightColor;

	float3 finalColor		= saturate(dot(L, N) * diffuse) +specularColor;
	
	return float4(finalColor, 1.0f);
}

float G1V(float NdotV, float k){
	return 1.0f / (NdotV*(1.0f - k) + k);
}

float4 CookTorrance(float3 N, float3 lightDirection, float4 diffuse, float3 lightColor, float roughness, float F0){
	float3 L = normalize(lightDirection);
	float3 V = normalize(CameraDir);
	float3 H = normalize(L + V);
	float alpha = roughness * roughness;
	float dotNL = saturate(dot(N, L));
	float dotNV = saturate(dot(N, V));
	float dotNH = saturate(dot(N, H));
	float dotLH = saturate(dot(L, H));

	float F, D, vis;

	// D
	float alphaSqr = alpha * alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0f;
	D = alphaSqr / (pi * denom * denom);

	// F
	float dotLH5 = pow(1.0f - dotLH, 5);
	F = F0 + (1.0 - F0) * (dotLH5);

	// V
	float k = alpha / 2.0f;
	vis = G1V(dotNL,k) * G1V(dotNV,k);

	float specular = dotNL * D * F * vis;

	float3 finalColor = saturate(dot(L, N) * diffuse) + (specular * lightColor);

	return float4(finalColor, 1.0f);
}

float4 CookTorrance2(float3 N, float3 ligh, float4 diffuse, float3 lightColor, float roughness, float F0){

	// Sample the textures
	float3 Normal		= N;
	float3 Specular		= float3(1, 1, 1);
	float2 Roughness = float2(roughness, F0);

	roughness.r *= 3;
  
	// Correct the input and compute aliases
	float3 ViewDir			= normalize(CameraDir);
		float3 LightDirection = normalize(ligh);
	float3 vHalf = normalize(LightDirection + ViewDir);
	float NormalDotHalf		= dot(Normal, vHalf);
	float ViewDotHalf		= dot(vHalf,  ViewDir);
	float NormalDotView		= dot(Normal, ViewDir);
	float NormalDotLight = dot(Normal, LightDirection);
  
	// Compute the geometric term
	float  G1          = ( 2.0f * NormalDotHalf * NormalDotView ) / ViewDotHalf;
	float  G2          = ( 2.0f * NormalDotHalf * NormalDotLight ) / ViewDotHalf;
	float  G           = min( 1.0f, max( 0.0f, min( G1, G2 ) ) );
  
	// Compute the fresnel term
	float  F          = Roughness.g + ( 1.0f - Roughness.g ) * pow( 1.0f - NormalDotView, 5.0f );
  
	// Compute the roughness term
	float  R_2        = Roughness.r * Roughness.r;
	float  NDotH_2    = NormalDotHalf * NormalDotHalf;
	float  A          = 1.0f / ( 4.0f * R_2 * NDotH_2 * NDotH_2 );
	float  B          = exp( -( 1.0f - NDotH_2 ) / ( R_2 * NDotH_2 ) );
	float  R          = A * B;
  
	// Compute the final term
	float3  S		= Specular * ( ( G * F * R ) / ( NormalDotLight * NormalDotView ) );
	float3  Final	= lightColor * max(0.0f, NormalDotLight) * (diffuse + S);
  
	return float4( Final, 1.0f );
}

float4 CookTorrance3(float3 N, float3 light, float4 diffuse, float3 lightColor, float roughnessValue, float F0){
	// set important material values
	float k = .2; // fraction of diffuse reflection (specular reflection = 1 - k)

	// interpolating normals will change the length of the normal, so renormalize the normal.
	float3 normal = N;
	float3 lightDirection = normalize(light);

	// do the lighting calculation for each fragment.
	float NdotL = max(dot(normal, lightDirection), 0.0);

	float specular = 0.0;
	if (NdotL > 0.0)
	{
		float3 eyeDir = normalize(CameraDir);

		// calculate intermediary values
		float3 halfVector = normalize(lightDirection + eyeDir);
		float NdotH = max(dot(normal, halfVector), 0.0);
		float NdotV = max(dot(normal, eyeDir), 0.0); // note: this could also be NdotL, which is the same value
		float VdotH = max(dot(eyeDir, halfVector), 0.0);
		float mSquared = roughnessValue * roughnessValue;

		// geometric attenuation
		float NH2 = 2.0 * NdotH;
		float g1 = (NH2 * NdotV) / VdotH;
		float g2 = (NH2 * NdotL) / VdotH;
		float geoAtt = min(1.0, min(g1, g2));

		// roughness (or: microfacet distribution function)
		// beckmann distribution function
		float r1 = 1.0 / (4.0 * mSquared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
		float roughness = r1 * exp(r2);

		// fresnel
		// Schlick approximation
		float fresnel = pow(1.0 - VdotH, 5.0);
		fresnel *= (1.0 - F0);
		fresnel += F0;

		specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14);
	}

	float3 finalValue = lightColor * NdotL * (k + specular * (1.0 - k));
	return float4(finalValue, 1.0);
}

float4 CookTorrance4(float3 normal, float3 light, float4 diffuse, float3 lightColor, float roughness_value, float ref_at_norm_incidence){
	// Compute any aliases and intermediary values
    // -------------------------------------------
	float3 half_vector = normalize(normalize(light) + CameraDir);
		float NdotL = saturate(dot(normal, normalize(light)));
    float NdotH        = saturate( dot( normal, half_vector ) );
    float NdotV        = saturate( dot( normal, CameraDir ) );
    float VdotH        = saturate( dot( CameraDir, half_vector ) );
    float r_sq         = roughness_value * roughness_value;
 
 
 
    // Evaluate the geometric term
    // --------------------------------
    float geo_numerator   = 2.0f * NdotH;
    float geo_denominator = VdotH;
 
    float geo_b = (geo_numerator * NdotV ) / geo_denominator;
    float geo_c = (geo_numerator * NdotL ) / geo_denominator;
    float geo   = min( 1.0f, min( geo_b, geo_c ) );
 
 
 
    // Now evaluate the roughness term
    // -------------------------------
    float roughness;
 
      
        float roughness_a = 1.0f / ( 4.0f * r_sq * pow( NdotH, 4 ) );
        float roughness_b = NdotH * NdotH - 1.0f;
        float roughness_c = r_sq * NdotH * NdotH;
 
        roughness = roughness_a * exp( roughness_b / roughness_c );
   
    // Next evaluate the Fresnel value
    // -------------------------------
    float fresnel = pow( 1.0f - VdotH, 5.0f );
    fresnel *= ( 1.0f - ref_at_norm_incidence );
    fresnel += ref_at_norm_incidence;
 
 
 
    // Put all the terms together to compute
    // the specular term in the equation
    // -------------------------------------
    float3 Rs_numerator   = ( fresnel * geo * roughness );
    float Rs_denominator  = NdotV * NdotL;
    float3 Rs             = Rs_numerator/ Rs_denominator;
 
 
 
    // Put all the parts together to generate
    // the final colour
    // --------------------------------------
    float3 final = max(0.0f, NdotL) * (float3(1,1,1)* Rs + diffuse); 
 
 
 
    // Return the result
    // -----------------
    return float4( final, 1.0f );
}

float4 P_Shader(VS_Out input) : SV_TARGET
{
	float4 diffuse = DiffuseTexture.Sample(TextureSampler, input.texUV);
	float3 normal = SampleNormalMap(input.normal, input.tangent, input.texUV);
	//float4 diffuse = float4(0.831, 0.6862, 0.21568, 1);
	//float3 normal = normalize(input.normal);


	//return SolidColor(float3(1, 0, 1));
	if (Mode.x == 0.0f) return CookTorrance(normal, input.lightDir, diffuse, float3(1, 1, 1), .13, .6);
	if (Mode.x == 1.0f) return CookTorrance2(normal, input.lightDir, diffuse, float3(1, 1, 1), 0.23, .1);
	if (Mode.x == 2.0f) return CookTorrance3(normal, input.lightDir, diffuse, float3(1, 1, 1), 0.23, .1);
	if (Mode.x == 3.0f) return CookTorrance4(normal, input.lightDir, diffuse, float3(1, 1, 1), 0.3, 1);
	//return CookTorrance4(normal, input.lightDir, diffuse, float3(1, 1, 1), 0.23, 0.1);
	//return diffuse;

	return BlinnPhong(normal, diffuse, float3(1, 1, 1), input.lightDir);
}
