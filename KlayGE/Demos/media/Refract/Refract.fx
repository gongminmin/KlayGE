#include "../Common/util.fx"

float4x4 model;
float4x4 mvp;
float4x4 mv;
float4x4 modelit;
float4x4 inv_vp;
float3 eye_pos;

float3 eta_ratio;

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

void RefractVS(float4 pos			: POSITION,
				float3 normal		: NORMAL,
				out float4 oPos		: POSITION,
				out float3 oNormal	: TEXCOORD0,
				out float3 oIncident : TEXCOORD1,
				out float3 refract_vec : TEXCOORD2,
				out float4 pos_ws	: TEXCOORD3,
				out float4 pos_es	: TEXCOORD4, 
				out float4 pos_ss	: TEXCOORD5)
{
	float4 pos_in_world = mul(pos, model);
	oPos = mul(pos, mvp);

	pos_ws = pos_in_world;
	pos_es = mul(pos, mv);
	pos_ss = oPos;

	oNormal = normalize(mul(normal, (float3x3)modelit));
	oIncident = normalize(pos_in_world.xyz - eye_pos);
	refract_vec = refract(oIncident, oNormal, eta_ratio.g);
}


sampler skybox_YcubeMapSampler;
sampler skybox_CcubeMapSampler;

sampler BackFace_Sampler;

half3 GlassReflect(float3 incident, float3 normal)
{
	half3 reflect_vec = reflect(incident, normal);
	half3 reflected_clr = decode_hdr_yc(texCUBE(skybox_YcubeMapSampler, reflect_vec).r,
					texCUBE(skybox_CcubeMapSampler, reflect_vec).ga);
					
	return reflected_clr;
}

half3 GlassRefract(float3 incident, float3 normal, float3 eta_ratio)
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

	half3 refracted_clr;
	refracted_clr.rbg = refracted_y.rbg * half3(refracted_c.xy, (1 - refracted_c.z - refracted_c.w))
											/ half3(0.299f, 0.114f, 0.587f);
												
	return refracted_clr;
}

float4 Refract20PS(float3 normal			: TEXCOORD0,
					float3 incident		: TEXCOORD1) : COLOR
{
	half3 refracted_clr = GlassRefract(incident, normal, eta_ratio);
	half3 reflected_clr = GlassReflect(incident, normal);

	half fresnel_factor = fast_fresnel(-incident, normal, 0.0977f);
	return float4(lerp(refracted_clr, reflected_clr, fresnel_factor), 1);
}

float4 Refract2xPS(float3 normal			: TEXCOORD0,
					float3 incident		: TEXCOORD1,
					float3 refract_vec	: TEXCOORD2,
					float3 pos_ws		: TEXCOORD3,
					float3 pos_es		: TEXCOORD4,
					float4 pos_ss		: TEXCOORD5) : COLOR
{
	half2 tex = pos_ss.xy / pos_ss.w;
	tex.y = -tex.y;
	tex = tex / 2 + 0.5f;
	half d = tex2D(BackFace_Sampler, tex).w - length(pos_es.xyz);

	half4 dir_ss = mul(half4(d * refract_vec, 0), mvp);
	pos_ss += dir_ss;

	tex = pos_ss.xy / pos_ss.w;
	tex.y = -tex.y;
	tex = tex / 2 + 0.5f;
	half3 back_face_normal = -tex2D(BackFace_Sampler, tex).xyz;

	half3 refracted_clr = GlassRefract(refract_vec, back_face_normal, eta_ratio);
	half3 reflected_clr = GlassReflect(incident, normal);

	half fresnel_factor = fast_fresnel(-incident, normal, 0.0977f);
	return float4(lerp(refracted_clr, reflected_clr, fresnel_factor), 1);
}

technique Refract20
{
	pass p0
	{
		ZFunc = LessEqual;
		CullMode = CCW;

		VertexShader = compile vs_1_1 RefractVS();
		PixelShader = compile ps_2_0 Refract20PS();
	}
}

technique Refract2x
{
	pass p0
	{
		ZFunc = LessEqual;
		CullMode = CCW;

		VertexShader = compile vs_1_1 RefractVS();
		PixelShader = compile ps_2_b Refract2xPS();
	}
}


void RefractBackFaceVS(float4 pos : POSITION,
							float3 normal : NORMAL,
							out float4 oPos : POSITION,
							out float3 oNormal : TEXCOORD0,
							out float3 pos_es : TEXCOORD1)
{
	oPos = mul(pos, mvp);
	oNormal = mul(normal, (float3x3)modelit);
	pos_es = mul(pos, model).xyz - eye_pos;
}

float4 RefractBackFacePS(float3 normal : TEXCOORD0,
							float3 pos_es : TEXCOORD1) : COLOR
{
	return float4(normalize(normal), length(pos_es));
}

technique RefractBackFace
{
	pass p0
	{
		ZFunc = Greater;
		CullMode = CW;

		VertexShader = compile vs_1_1 RefractBackFaceVS();
		PixelShader = compile ps_2_0 RefractBackFacePS();
	}
}
