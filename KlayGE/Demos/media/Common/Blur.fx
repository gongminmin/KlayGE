sampler src_sampler;

float color_weight[15];
float tex_coord_offset[15];

void BlurVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

float4 BlurXPS(float2 inTex: TEXCOORD0) : COLOR0
{
	half4 color = half4(0, 0, 0, 1);

	for (int i = 0; i < 15; ++ i)
	{
		color.rgb += tex2D(src_sampler, inTex + float2(tex_coord_offset[i], 0)).rgb * half(color_weight[i]);
	}

	return color;
}

float4 BlurYPS(float2 inTex: TEXCOORD0) : COLOR0
{
	half4 color = float4(0, 0, 0, 1);

	for (int i = 0; i < 15; ++ i)
	{
		color.rgb += tex2D(src_sampler, inTex + float2(0, tex_coord_offset[i])).rgb * half(color_weight[i]);
	}

	return color;
}

technique BlurX
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_1_1 BlurVS();
		PixelShader = compile ps_2_0 BlurXPS();
	}
}

technique BlurY
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_1_1 BlurVS();
		PixelShader = compile ps_2_0 BlurYPS();
	}
}

