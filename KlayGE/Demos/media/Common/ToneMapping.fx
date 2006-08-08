const float MIDDLE_GREY = 0.36f;

void ToneMappingVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

sampler src_sampler;
sampler lum_sampler;
sampler bloom_sampler;

float4 ToneMappingPS(float2 oTex : TEXCOORD0) : COLOR
{
	half3 blur = tex2D(bloom_sampler, oTex).rgb;
	half lum = max(0.001f, tex2D(lum_sampler, float2(0.5f, 0.5f)).r);
	
	half3 clr = tex2D(src_sampler, oTex) + blur * 0.25f;

	clr *= MIDDLE_GREY / lum;
	clr /= 1.0f + clr;

	return float4(clr, 1);
}

technique ToneMapping
{
	pass p0
	{
		CullMode = CCW;
		ZEnable = false;

		VertexShader = compile vs_1_1 ToneMappingVS();
		PixelShader = compile ps_2_0 ToneMappingPS();
	}
}

