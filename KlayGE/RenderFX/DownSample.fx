const float BRIGHT_THRESHOLD = 0.5f;

void DownsampleVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oPos.z = 0.9f;
	oTex = tex;
}

sampler src_sampler;
float4 tex_coord_offset[4];

half4 SuppressLDR(half4 c)
{
	if ((c.r > BRIGHT_THRESHOLD) || (c.g > BRIGHT_THRESHOLD) || (c.b > BRIGHT_THRESHOLD))
	{
		return c;
	}
	else
	{
		return 0;
	}
}

float4 DownsamplePS(float2 oTex : TEXCOORD0) : COLOR
{
	half4 clr = 0;
	for (int i = 0; i < 4; ++ i)
	{
		clr += SuppressLDR(tex2D(src_sampler, oTex + tex_coord_offset[i]) * 1.0001f);
	}

	return clr / 4;
}

technique Downsample
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 DownsampleVS();
		PixelShader = compile ps_2_0 DownsamplePS();
	}
}
