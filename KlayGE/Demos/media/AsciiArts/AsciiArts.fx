struct VS_INPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
};

VS_OUTPUT AsciiArtsVS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.pos = input.pos;
	output.pos.z = 0.9f;
	output.texcoord0 = input.texcoord0;

	return output;
}

sampler src_sampler;
sampler lums_sampler;
float2 cell_per_row_line;

float4 AsciiArtsPS(float2 tex_coord0 : TEXCOORD0) : COLOR
{
	const float3 rgb_to_lum = float3(0.299, 0.587, 0.114);

	float lum = dot(tex2D(src_sampler, tex_coord0).rgb, rgb_to_lum);
	float2 t = float2(floor(lum * 31) / 32, 0) + frac(tex_coord0 / cell_per_row_line) / float2(32, 1);
	return lum * tex2D(lums_sampler, t);
}

technique AsciiArts
{
	pass p0
	{
		VertexShader = compile vs_1_1 AsciiArtsVS();
		PixelShader = compile ps_2_0 AsciiArtsPS();
	}
}
