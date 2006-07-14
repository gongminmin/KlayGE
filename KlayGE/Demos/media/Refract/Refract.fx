#include "../Common/util.fx"

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
	
	float3 normal        : TEXCOORD0;
	float3 incident      : TEXCOORD1;
};

// fresnel approximation
half fast_fresnel(half3 eye, half3 normal, half R0)
{
	// R0 = pow(1.0 - refractionIndexRatio, 2.0) / pow(1.0 + refractionIndexRatio, 2.0);

	half edn = max(0, dot(eye, normal));
	return R0 + (1.0 - R0) * pow(1.0 - edn, 5);
}

float3x3 my_refract3(float3 i, float3 n, float3 eta)
{
    float cosi = dot(-i, n);
    float3 cost2 = 1.0 - eta * eta * (1.0 - cosi * cosi);
    float3 tmp = eta * cosi - sqrt(abs(cost2));
    float3 t0 = eta.x * i + tmp.x * n;
    float3 t1 = eta.y * i + tmp.y * n;
    float3 t2 = eta.z * i + tmp.z * n;
    bool3 b = cost2 > 0.0f;
    
    float3x3 ret;
    ret[0] = t0 * b.x;
    ret[1] = t1 * b.y;
    ret[2] = t2 * b.z;
    return ret;
}

VS_OUTPUT RefractVS(VS_INPUT input)
{
	VS_OUTPUT output;
	
	float4 pos_in_world = mul(input.pos, model);
	output.pos = mul(pos_in_world, mvp);

	float3 normal = normalize(mul(input.normal, (float3x3)modelit));
	float3 incident = normalize(pos_in_world.xyz - eyePos);

	output.normal = normal;
	output.incident = incident;
	
	return output;
}


sampler skybox_YcubeMapSampler;
sampler skybox_CcubeMapSampler;

float4 RefractPS(float3 normal        : TEXCOORD0,
					float3 incident   : TEXCOORD1) : COLOR
{
	float3x3 refract_rgb = my_refract3(incident, normal, eta_ratio);
	
	half3 refracted_y = half3(texCUBE(skybox_YcubeMapSampler, refract_rgb[0]).r,
									texCUBE(skybox_YcubeMapSampler, refract_rgb[1]).r,
									texCUBE(skybox_YcubeMapSampler, refract_rgb[2]).r);
	refracted_y = exp2(refracted_y * 65536 / 2048 - 16);
	half4 refracted_c = half4(texCUBE(skybox_CcubeMapSampler, refract_rgb[0]).a,
									texCUBE(skybox_CcubeMapSampler, refract_rgb[1]).g,
									texCUBE(skybox_CcubeMapSampler, refract_rgb[2]).ga);
	refracted_c *= refracted_c;

	half4 refracted_clr;
	refracted_clr.rbg = refracted_y.rbg * half3(refracted_c.xy, (1 - refracted_c.z - refracted_c.w))
											/ half3(0.299f, 0.114f, 0.587f);
	refracted_clr.a = 1;
	
	float3 reflect_vec = reflect(incident, normal);
	half4 reflected_clr = decode_hdr_yc(texCUBE(skybox_YcubeMapSampler, reflect_vec).r,
					texCUBE(skybox_CcubeMapSampler, reflect_vec).ga);

	float fresnel_factor = fast_fresnel(-incident, normal, 0.03f);
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
