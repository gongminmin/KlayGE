void SumLumVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oPos.z = 0.9f;
	oTex = tex;
}

sampler src_sampler;
float4 tex_coord_offset[2];

float4 SumLum4x4LogPS(float2 oTex : TEXCOORD0) : COLOR
{
	const half3 rgb_to_lum = half3(0.27, 0.67, 0.06);

	half s = 0;
	for (int i = 0; i < 2; ++ i)
	{
		s += log(dot(tex2D(src_sampler, oTex + tex_coord_offset[i].xy).rgb, rgb_to_lum) + 0.0001f);
		s += log(dot(tex2D(src_sampler, oTex + tex_coord_offset[i].zw).rgb, rgb_to_lum) + 0.0001f);
	}
	
	return s / 4;
}

float4 SumLum4x4PS(float2 oTex : TEXCOORD0) : COLOR
{
	half s = 0;
	for (int i = 0; i < 2; ++ i)
	{
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].xy).r;
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].zw).r;
	}

	return s / 4;
}

float4 SumLum4x4ExpPS(float2 oTex : TEXCOORD0) : COLOR
{
	half s = 0;
	for (int i = 0; i < 2; ++ i)
	{
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].xy).r;
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].zw).r;
	}

	return exp(s / 4);
}

sampler last_lum_sampler;
float frame_delta;
float4 AdaptedLumPS(float2 oTex : TEXCOORD0) : COLOR
{
	float adaptedLum = tex2D(last_lum_sampler, float2(0.5f, 0.5f)).r;
	float currentLum = tex2D(src_sampler, float2(0.5f, 0.5f)).r;

	return adaptedLum + (currentLum - adaptedLum) * (1 - pow(0.98f, 50 * frame_delta));
}


technique SumLumLog
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 SumLumVS();
		PixelShader = compile ps_2_0 SumLum4x4LogPS();
	}
}

technique SumLum
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 SumLumVS();
		PixelShader = compile ps_2_0 SumLum4x4PS();
	}
}

technique SumLumExp
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 SumLumVS();
		PixelShader = compile ps_2_0 SumLum4x4ExpPS();
	}
}

technique AdaptedLum
{
	pass p0
	{
		VertexShader = compile vs_1_1 SumLumVS();
		PixelShader = compile ps_2_0 AdaptedLumPS();
	}
}
