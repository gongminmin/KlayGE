// Gaussian filter to blur along x-axis.

float4x4 matMVP : WorldViewProjection;

float Width;
float Height;

float PixelWeight[8] = { 0.2537, 0.2185, 0.0821, 0.0461, 0.0262, 0.0162, 0.0102, 0.0052 };

sampler2D BlurXSampler;
sampler2D BlurYSampler;

struct VS_OUT
{
	float4 Pos:	POSITION;
	float2 Tex:	TEXCOORD0;
};

VS_OUT BlurVS(float4 inPos: POSITION, float2 inTex : TEXCOORD0)
{
	VS_OUT OUT;

	// Output the transformed vertex
	OUT.Pos = mul(inPos, matMVP);

	// Output the texture coordinates
	OUT.Tex = inTex;

	return OUT;
}

float4 BlurPS(float2 inTex: TEXCOORD0, float2 offset, sampler BlurSampler) : COLOR0
{
	float4 color = tex2D(BlurXSampler, inTex);

	// Sample pixels on either side
	for (int i = 0; i < 8; ++ i)
	{
		color += tex2D(BlurSampler, inTex + offset * i) * PixelWeight[i];
		color += tex2D(BlurSampler, inTex - offset * i) * PixelWeight[i];
	}

	return color;
}

float4 BlurXPS(float2 inTex: TEXCOORD0,
		uniform sampler2D BlurXSampler) : COLOR0
{
	return BlurPS(inTex, float2(1.0 / Width, 0), BlurXSampler);
}

float4 BlurYPS(float2 inTex: TEXCOORD0,
		uniform sampler2D BlurYSampler) : COLOR0
{
	return BlurPS(inTex, float2(0, 1.0 / Height), BlurYSampler);
}


technique BlurXTechnique
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 BlurVS();
		PixelShader  = compile ps_2_0 BlurXPS(BlurXSampler);
	}
}

technique BlurYTechnique
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 BlurVS();
		PixelShader  = compile ps_2_0 BlurYPS(BlurYSampler);
	}
}
