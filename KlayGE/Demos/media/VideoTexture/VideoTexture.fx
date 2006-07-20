float4x4 mvp;

sampler video_sampler;

void ObjectVS(float4 Position : POSITION,
					float2 Tex0 : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex0 : TEXCOORD0)
{
	oPos = mul(Position, mvp);
	oTex0 = Tex0;
}

float4 ObjectPS(float2 Tex0 : TEXCOORD0) : COLOR 
{
	return tex2D(video_sampler, Tex0);
}

technique Object
{
	pass p0
	{
		CullMode = CCW;

		VertexShader = compile vs_1_1 ObjectVS();
		PixelShader = compile ps_1_1 ObjectPS();
	}
}