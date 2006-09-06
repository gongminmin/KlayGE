sampler src_sampler;
float4 tex_coord_offset[2];

void SumLumVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float4 oTex[2] : TEXCOORD)
{
	oPos = pos;

	for (int i = 0; i < 2; ++ i)
	{
		oTex[i] = tex.xyxy + tex_coord_offset[i];
	}
}

float4 SumLum4x4LogPS(float4 iTex[2] : TEXCOORD) : COLOR
{
	const half3 RGB_TO_LUM = half3(0.27, 0.67, 0.06);

	half s = 0;
	for (int i = 0; i < 2; ++ i)
	{
		s += log(dot(tex2D(src_sampler, iTex[i].xy).rgb, RGB_TO_LUM) + 0.001f);
		s += log(dot(tex2D(src_sampler, iTex[i].zw).rgb, RGB_TO_LUM) + 0.001f);
	}

	return s / 4;
}

float4 SumLum4x4IterativePS(float4 iTex[2] : TEXCOORD) : COLOR
{
	half s = 0;
	for (int i = 0; i < 2; ++ i)
	{
		s += tex2D(src_sampler, iTex[i].xy).r;
		s += tex2D(src_sampler, iTex[i].zw).r;
	}

	return s / 4;
}

sampler last_lum_sampler;
float frame_delta;
float4 AdaptedLumPS(float2 oTex : TEXCOORD0) : COLOR
{
	float adapted_lum = tex2D(last_lum_sampler, float2(0.5f, 0.5f)).r;
	float current_lum = exp(tex2D(src_sampler, float2(0.5f, 0.5f)).r);

	return adapted_lum + (current_lum - adapted_lum) * (1 - pow(0.98f, 50 * frame_delta));
}


technique SumLumLog
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_2_0 SumLumVS();
		PixelShader = compile ps_2_0 SumLum4x4LogPS();
	}
}

technique SumLumIterative
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_2_0 SumLumVS();
		PixelShader = compile ps_2_0 SumLum4x4IterativePS();
	}
}

technique AdaptedLum
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;
		
		VertexShader = compile vs_2_0 SumLumVS();
		PixelShader = compile ps_2_0 AdaptedLumPS();
	}
}
