const float EXPOSURE = 0.55f;

void ToneMappingVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oPos.z = 0.9f;
	oTex = tex;
}

sampler lum_sampler;
sampler scene_sampler;
sampler bloom_sampler;

float4 ToneMappingPS(float2 oTex : TEXCOORD0) : COLOR
{
	half3 blur = tex2D(bloom_sampler, oTex).rgb;
	half lum = max(0.001f, tex2D(lum_sampler, float2(0.5f, 0.5f)).r);
	
	half3 clr = tex2D(scene_sampler, oTex) + blur * 0.25f;

	half3 L = clr * EXPOSURE / lum;
	clr = L / (1 + L);

    return float4(clr, 1);
}

technique ToneMapping
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 ToneMappingVS();
		PixelShader = compile ps_2_0 ToneMappingPS();
	}
}

