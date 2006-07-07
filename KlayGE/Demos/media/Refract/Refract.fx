float4 decode_hdr_yc(float y, float2 c)
{
	float Y = exp2(y * 65536 / 2048 - 16);
	float2 C = c;
	C *= C;
	
	return float4(Y * float3(C.g, (1 - C.g - C.r), C.r) / float3(0.299f, 0.587f, 0.114f), 1);
}

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
	
	float3 refract_r     : TEXCOORD0;
	float3 refract_g     : TEXCOORD1;
	float3 refract_b     : TEXCOORD2;
	float3 reflect_vec   : TEXCOORD3;
	float fresnel_factor : TEXCOORD4;
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

	float3 normal = normalize(mul(input.normal, (float3x3)modelit));
	float3 incident = normalize(pos_in_world.xyz - eyePos);

	output.refract_r = refract(incident, normal, eta_ratio.x);
	output.refract_g = refract(incident, normal, eta_ratio.y);
	output.refract_b = refract(incident, normal, eta_ratio.z);
	output.reflect_vec = reflect(incident, normal);
	output.fresnel_factor = fast_fresnel(-incident, normal, 0.03f);
	
	return output;
}


sampler skybox_YcubeMapSampler;
sampler skybox_CcubeMapSampler;

float4 RefractPS(float3 refract_r     : TEXCOORD0,
					float3 refract_g     : TEXCOORD1,
					float3 refract_b     : TEXCOORD2,
					float3 reflect_vec   : TEXCOORD3,
					float fresnel_factor : TEXCOORD4) : COLOR
{
	half3 refracted_y = half3(texCUBE(skybox_YcubeMapSampler, refract_r).r,
									texCUBE(skybox_YcubeMapSampler, refract_g).r,
									texCUBE(skybox_YcubeMapSampler, refract_b).r);
	refracted_y = exp2(refracted_y * 65536 / 2048 - 16);
	half4 refracted_c = half4(texCUBE(skybox_CcubeMapSampler, refract_r).a,
									texCUBE(skybox_CcubeMapSampler, refract_g).g,
									texCUBE(skybox_CcubeMapSampler, refract_b).ga);
	refracted_c *= refracted_c;

	half4 refracted_clr;
	refracted_clr.rbg = refracted_y.rbg * half3(refracted_c.xy, (1 - refracted_c.z - refracted_c.w))
											/ half3(0.299f, 0.114f, 0.587f);
	refracted_clr.a = 1;

	half4 reflected_clr = decode_hdr_yc(texCUBE(skybox_YcubeMapSampler, reflect_vec).r,
					texCUBE(skybox_CcubeMapSampler, reflect_vec).ga);

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
