sampler src_sampler;

float color_weight[8];
float tex_coord_offset[8];

void BlurXVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float4 oTex[4] : TEXCOORD)
{
	oPos = pos;

	for (int i = 0; i < 4; ++ i)
	{
		oTex[i] = tex.xyxy + float4(tex_coord_offset[i * 2 + 0], 0, tex_coord_offset[i * 2 + 1], 0);
	}
}

void BlurYVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float4 oTex[4] : TEXCOORD)
{
	oPos = pos;

	for (int i = 0; i < 4; ++ i)
	{
		oTex[i] = tex.xyxy + float4(0, tex_coord_offset[i * 2 + 0], 0, tex_coord_offset[i * 2 + 1]);
	}
}

float4 BlurPS(float4 iTex[4] : TEXCOORD) : COLOR0
{
	half4 color = half4(0, 0, 0, 1);

	for (int i = 0; i < 4; ++ i)
	{
		color.rgb += tex2D(src_sampler, iTex[i].xy).rgb * color_weight[i * 2 + 0];
		color.rgb += tex2D(src_sampler, iTex[i].zw).rgb * color_weight[i * 2 + 1];
	}

	return color;
}

technique BlurX
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_2_0 BlurXVS();
		PixelShader = compile ps_2_0 BlurPS();
	}
}

technique BlurY
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_2_0 BlurYVS();
		PixelShader = compile ps_2_0 BlurPS();
	}
}

