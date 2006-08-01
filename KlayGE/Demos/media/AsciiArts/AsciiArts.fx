void PostProcessVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

sampler src_sampler;
sampler lums_sampler;
float2 cell_per_row_line;

float4 AsciiArtsPS(float2 tex_coord0 : TEXCOORD0) : COLOR
{
	half lum = tex2D(src_sampler, tex_coord0).r;
	half2 t = half2(floor(lum * 31) / 32, 0) + frac(tex_coord0 / cell_per_row_line) / half2(32, 1);
	return lum * tex2D(lums_sampler, t);
}

technique AsciiArts
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;
		
		VertexShader = compile vs_1_1 PostProcessVS();
		PixelShader = compile ps_2_0 AsciiArtsPS();
	}
}


float4 tex_coord_offset[8];

float4 Downsample8x8PS(float2 oTex : TEXCOORD0) : COLOR
{
	const half3 rgb_to_lum = half3(0.299, 0.587, 0.114);

	half4 s = 0;
	for (int i = 0; i < 8; ++ i)
	{
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].xy);
		s += tex2D(src_sampler, oTex + tex_coord_offset[i].zw);
	}

	return dot(s.rgb / 16, rgb_to_lum);
}

technique Downsample8x8
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_1_1 PostProcessVS();
		PixelShader = compile ps_2_0 Downsample8x8PS();
	}
}
