float4x4 model;
float4x4 mvp;
float4x4 modelit;
float3 eyePos;

float3 eta_ratio;
float exposure_level;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float3 normal		: NORMAL;
};

struct VS_OUTPUT
{
	float4 pos			 : POSITION;
	
	float3 normal		 : TEXCOORD0;
	float3 incident		 : TEXCOORD1;
};

// fresnel approximation
half fast_fresnel(half3 eye, half3 normal, half R0)
{
	// R0 = pow(1.0 - refractionIndexRatio, 2.0) / pow(1.0 + refractionIndexRatio, 2.0);

	half edn = max(0, dot(eye, normal));
	return R0 + (1.0 - R0) * pow(1.0 - edn, 5);
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

	output.normal = normalize(mul(input.normal, (float3x3)modelit));
	output.incident = normalize(pos_in_world.xyz - eyePos);

	return output;
}


samplerCUBE cubeMapSampler;

float4 RefractPS(float3 normal : TEXCOORD0,
					float3 incident : TEXCOORD1) : COLOR
{
	half3 TRed = refract(incident, normal, eta_ratio.x);
	half3 TGreen = refract(incident, normal, eta_ratio.y);
	half3 TBlue = refract(incident, normal, eta_ratio.z);

	half4 refracted_clr;
	refracted_clr.r = texCUBE(cubeMapSampler, TRed).r;
	refracted_clr.g = texCUBE(cubeMapSampler, TGreen).g;
	refracted_clr.b = texCUBE(cubeMapSampler, TBlue).b;
	refracted_clr.a = 1;

	half fresnel_factor = fast_fresnel(-incident, normal, 0.03f);

	half3 R = reflect(incident, normal);
	half4 reflected_clr = texCUBE(cubeMapSampler, R);

	return lerp(refracted_clr, reflected_clr, fresnel_factor) * exposure_level;
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
