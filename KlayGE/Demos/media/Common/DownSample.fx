const float BRIGHT_THRESHOLD = 0.5f;

void DownsampleVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

sampler src_sampler;

half4 BrightPass(half4 c)
{
	if (any(c.rgb > BRIGHT_THRESHOLD))
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
	return BrightPass(tex2D(src_sampler, oTex));
}

technique Downsample
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_1_1 DownsampleVS();
		PixelShader = compile ps_2_0 DownsamplePS();
	}
}
