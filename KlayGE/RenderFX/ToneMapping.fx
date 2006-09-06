const float MIDDLE_GREY = 0.36f;

sampler src_sampler;
sampler lum_sampler;
sampler bloom_sampler;


float4 ToneMapping(float3 color, float3 blur, float adapted_lum)
{
	const half3 RGB_TO_LUM = half3(0.27, 0.67, 0.06);
	const half3 BLUE_SHIFT = half3(1.05, 0.97, 1.27); 

	color += blur * 0.25f;

	// Blue shift
	half blue_shift_coef = 1.0f - (adapted_lum + 1.5f) / 4.1f;
	half3 rod_clr = dot(color, RGB_TO_LUM) * BLUE_SHIFT;
	color = lerp(color, rod_clr, saturate(blue_shift_coef));

	// Tone mapping
	color *= MIDDLE_GREY / adapted_lum;
	color /= 1.0f + color;

	return float4(color, 1);
}


void ToneMapping30VS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float3 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex.xy = tex;
	oTex.z = max(0.001f, tex2Dlod(lum_sampler, float4(0.5f, 0.5f, 0, 0)).r);
}

float4 ToneMapping30PS(float3 iTex : TEXCOORD0) : COLOR
{
	return ToneMapping(tex2D(src_sampler, iTex.xy), tex2D(bloom_sampler, iTex.xy), iTex.z);
}

technique ToneMapping30
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_3_0 ToneMapping30VS();
		PixelShader = compile ps_3_0 ToneMapping30PS();
	}
}


void ToneMapping20VS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

float4 ToneMapping20PS(float2 iTex : TEXCOORD0) : COLOR
{
	half lum = max(0.001f, tex2D(lum_sampler, float2(0.5f, 0.5f)).r);
	return ToneMapping(tex2D(src_sampler, iTex), tex2D(bloom_sampler, iTex), lum);
}

technique ToneMapping20
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_2_0 ToneMapping20VS();
		PixelShader = compile ps_2_0 ToneMapping20PS();
	}
}

