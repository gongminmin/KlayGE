float4x4 matMVP : WorldViewProjection;

float Width;
float Height;

// This hack is necessary because the suppress function
// seems to work only when Kd is greater than 1, even by 0.0001f!!!
float Kd = 1.0001f;

sampler2D DownSampler;

struct VS_OUT
{
	float4 Pos:	POSITION;
	float2 Tex:	TEXCOORD0;
};

VS_OUT vs_main( float4 inPos: POSITION, float2 inTex: TEXCOORD0 )
{
	VS_OUT OUT;

	// Output the transformed vertex
	OUT.Pos = mul(inPos, matMVP);

	// Output the texture coordinates
	OUT.Tex = inTex;

	return OUT;
}

float4 SuppressLDR(float4 c)
{
	if ((c.r > 1.0f) || (c.g > 1.0f) || (c.b > 1.0f))
	{
		return c;
	}
	else
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

float4 ps_main(float2 inTex: TEXCOORD0,
		uniform sampler2D DownSampler) : COLOR0
{
	return SuppressLDR(tex2D(DownSampler, inTex) * Kd);
}

technique Technique0
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main(DownSampler);
	}
}
