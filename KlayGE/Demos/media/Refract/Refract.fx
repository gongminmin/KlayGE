float4x4 model;
float4x4 mvp;
float4x4 modelit;
float3 eyePos;

float3 eta_ratio;
float3 fresnel_values;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float3 normal		: NORMAL;
};

struct VS_OUTPUT
{
	float4 pos			 : POSITION;
	
	float3 R			 : TEXCOORD0;
	float3 TRed			 : TEXCOORD1;
	float3 TGreen		 : TEXCOORD2;
	float3 TBlue		 : TEXCOORD3;
	float fresnel_factor : TEXCOORD4;
};

// fresnel approximation
float fast_fresnel(float3 incident, float3 normal, float3 fresnel_values)
{
    float power = fresnel_values.x;
    float scale = fresnel_values.y;
    float bias = fresnel_values.z;

    return bias + pow(1.0 + dot(incident, normal), power) * scale;
}

float3 my_refract(float3 i, float3 n, float eta)
{
    float cosi = dot(-i, n);
    float cost2 = 1.0 - eta * eta * (1.0 - cosi * cosi);
    float3 t = eta * i + ((eta * cosi - sqrt(abs(cost2))) * n);
    return t * (cost2 > 0.0);
}

VS_OUTPUT RefractVS(VS_INPUT input)
{
	VS_OUTPUT output;
	
	float4 pos_in_world = mul(input.pos, model);
	output.pos = mul(pos_in_world, mvp);

	float3 normal = normalize(mul(input.normal, (float3x3)modelit));
	float3 incident = normalize(pos_in_world.xyz - eyePos);
	
	output.R = reflect(incident, normal);

	output.TRed = refract(incident, normal, eta_ratio.x);
	output.TGreen = refract(incident, normal, eta_ratio.y);
	output.TBlue = refract(incident, normal, eta_ratio.z);

	output.fresnel_factor = fast_fresnel(incident, normal, fresnel_values);

	return output;
}


samplerCUBE cubeMapSampler;

float4 RefractPS(float3 R : TEXCOORD0,
					float3 TRed : TEXCOORD1,
					float3 TGreen : TEXCOORD2,
					float3 TBlue : TEXCOORD3,
					float fresnel_factor : TEXCOORD4) : COLOR
{
	float4 reflected_clr = texCUBE(cubeMapSampler, R);

	float4 refracted_clr;
	refracted_clr.r = texCUBE(cubeMapSampler, TRed).r;
	refracted_clr.g = texCUBE(cubeMapSampler, TGreen).g;
	refracted_clr.b = texCUBE(cubeMapSampler, TBlue).b;
	refracted_clr.a = 1;

	return lerp(refracted_clr, reflected_clr, fresnel_factor);
}

technique Refract
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 RefractVS();
		PixelShader = compile ps_2_0 RefractPS();
	}
}
