struct VS_INPUT
{
	float4 pos			: POSITION;
	float3 texcoord0	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float3 texcoord0	: TEXCOORD0;
};

VS_OUTPUT FractalVS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.pos = input.pos;
	output.texcoord0 = input.texcoord0;

	return output;
}

float4 FractalPS(float2 texCoord0 : TEXCOORD0,
					uniform int times) : COLOR
{
	float2 z = float2(0, 0);
	float2 c = texCoord0;

	for (int i = 0; i < times; ++ i)
	{
		z = float2(z.x * z.x - z.y * z.y, z.x * z.y + z.y * z.x) + c;
	}

	return float4(0, 0, length(z) / 2, 1);
}

technique FractalPS30
{
	pass p0
	{
		VertexShader = compile vs_3_0 FractalVS();
		PixelShader = compile ps_3_0 FractalPS(50);
	}
}

technique FractalPS2a
{
	pass p0
	{
		VertexShader = compile vs_1_1 FractalVS();
		PixelShader = compile ps_2_a FractalPS(30);
	}
}

technique FractalPS20
{
	pass p0
	{
		VertexShader = compile vs_1_1 FractalVS();
		PixelShader = compile ps_2_0 FractalPS(12);
	}
}

