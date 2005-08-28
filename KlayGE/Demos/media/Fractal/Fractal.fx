struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
	float2 texcoord1	: TEXCOORD1;
};

VS_OUTPUT FractalVS(float4 pos : POSITION, float2 texcoord0 : TEXCOORD0)
{
	VS_OUTPUT output;

	output.pos = pos;
	output.texcoord0 = texcoord0;
	output.texcoord1 = texcoord0 * 2.4 - float2(1.8, 1.2);

	return output;
}

sampler fractal_sampler;

float4 FractalPS(float2 texCoord0 : TEXCOORD0, float2 texCoord1 : TEXCOORD1,
					uniform sampler fractal_sampler) : COLOR
{
	float2 z = tex2D(fractal_sampler, texCoord0).rg;
	float2 c = texCoord1;

	z = float2(z.x * z.x - z.y * z.y, z.x * z.y + z.y * z.x) + c;

	return float4(z.xy, 0, 1);
}

technique Fractal
{
	pass p0
	{
		VertexShader = compile vs_1_1 FractalVS();
		PixelShader = compile ps_2_0 FractalPS(fractal_sampler);
	}
}

void ShowVS(float4 pos : POSITION, float2 texcoord0 : TEXCOORD0,
			out float4 oPos : POSITION, out float2 oTexcoord0 : TEXCOORD0)
{
	oPos = pos;
	oTexcoord0 = texcoord0;
}

float4 ShowPS(float2 texCoord0 : TEXCOORD0,
					uniform sampler fractal_sampler) : COLOR
{
	float2 z = tex2D(fractal_sampler, texCoord0).rg;
	return float4(0, 0, length(z) / 2, 1);
}

technique Show
{
	pass p0
	{
		VertexShader = compile vs_1_1 ShowVS();
		PixelShader = compile ps_2_0 ShowPS(fractal_sampler);
	}
}
