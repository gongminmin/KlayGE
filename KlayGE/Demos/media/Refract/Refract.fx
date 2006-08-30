#include "../Common/util.fx"

float4x4 model;
float4x4 mvp;
float4x4 modelit;
float4x4 inv_vp;
float3 eyePos;

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
				out float3 oRefract : TEXCOORD2,
				out float4 oPosWS	: TEXCOORD3,
				out float4 oPosSS	: TEXCOORD4)
{
	float4 pos_in_world = mul(pos, model);
	oPos = mul(pos, mvp);

	oNormal = normalize(mul(normal, (float3x3)modelit));
	oIncident = normalize(pos_in_world.xyz - eyePos);
	oRefract = refract(oIncident, oNormal, eta_ratio.g);
	oPosWS = pos_in_world;
	oPosSS = oPos;
}


sampler skybox_YcubeMapSampler;
sampler skybox_CcubeMapSampler;

sampler BackFace_Sampler;

float4 Refract20PS(float3 normal			: TEXCOORD0,
					float3 incident		: TEXCOORD1) : COLOR
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

	float fresnel_factor = fast_fresnel(-incident, normal, 0.0977f);
	return lerp(refracted_clr, reflected_clr, fresnel_factor);
}

float4 Refract2xPS(float3 normal			: TEXCOORD0,
					float3 incident		: TEXCOORD1,
					float3 refract_vec	: TEXCOORD2,
					float4 PosWS		: TEXCOORD3,
					float4 PosSS		: TEXCOORD4) : COLOR
{
	PosWS /= PosWS.w;
	PosSS /= PosSS.w;
	
	half2 Tex;
	Tex = PosSS.xy / 2 + 0.5f;
	Tex.y = 1 - Tex.y;
	half4 back_face_pos = mul(half4(PosSS.xy, tex2D(BackFace_Sampler, Tex).w, 1), inv_vp);
	back_face_pos /= back_face_pos.w;
	half3 d = length(PosWS.xyz - back_face_pos.xyz);
	half3 back_face_normal = normalize(tex2D(BackFace_Sampler, Tex + normal.xz * d * 0.05 / PosSS.z).xyz * 2 - 1);
	
	float3x3 refract_rgb = my_refract3(d * refract_vec, back_face_normal, eta_ratio);
	
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

	float fresnel_factor = fast_fresnel(-incident, normal, 0.0977f);
	return lerp(refracted_clr, reflected_clr, fresnel_factor);
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
							out float2 oDepth : TEXCOORD1)
{
	oPos = mul(pos, mvp);
	oNormal = mul(normal, (float3x3)modelit);
	oDepth = oPos.zw;
}

float4 RefractBackFacePS(float3 normal : TEXCOORD0,
							float2 depth : TEXCOORD1) : COLOR
{
	return float4(normal / 2 + 0.5f, depth.x / depth.y);
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
