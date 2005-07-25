float cell_per_row, cell_per_line;

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
	output.texcoord0 = input.texcoord0;

	return output;
}

texture scene_tex;
sampler2D scene_sampler = sampler_state
{
	Texture = <scene_tex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

texture lums_tex;
sampler3D lums_sampler = sampler_state
{
	Texture = <lums_tex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	AddressU  = Clamp;
	AddressV  = Clamp;
	AddressW  = Clamp;
};

float4 AsciiArtsPS(float2 tex_coord0	: TEXCOORD0,
					uniform sampler2D scene_sampler,
					uniform sampler3D lums_sampler,
					uniform float2 arg) : COLOR
{
	const float3 rgb_to_lum = float3(0.299, 0.587, 0.114);

	float lum = dot(tex2D(scene_sampler, tex_coord0).rgb, rgb_to_lum);
	float2 t = float2(frac(floor(lum * 255) / 32), 0) + frac(tex_coord0 / arg) / float2(32, 1);
	return tex3D(lums_sampler, float3(t, lum));
}

technique AsciiArts
{
	pass p0
	{
		VertexShader = compile vs_1_1 AsciiArtsVS();
		PixelShader = compile ps_2_0 AsciiArtsPS(scene_sampler, lums_sampler, float2(cell_per_row, cell_per_line));
	}
}
