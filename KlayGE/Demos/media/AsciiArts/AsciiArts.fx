float cell_per_row, cell_per_line;
float char_per_row;

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

VS_OUTPUT DownsampleVS(VS_INPUT input)
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
	AddressU  = Wrap;
	AddressV  = Wrap;
};

float4 DownsamplePS(float2 tex_coord0	: TEXCOORD0,
					uniform sampler2D scene_sampler) : COLOR
{
	return float4(tex2D(scene_sampler, tex_coord0).rgb, 1);
}

technique Downsample
{
	pass p0
	{
		VertexShader = compile vs_1_1 DownsampleVS();
		PixelShader = compile ps_1_1 DownsamplePS(scene_sampler);
	}
}

texture ascii_tex;
sampler2D ascii_sampler = sampler_state
{
	Texture = <ascii_tex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
};

texture lums_tex;
sampler1D lums_sampler = sampler_state
{
	Texture = <lums_tex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

float4 ShowAsciiPS(float2 tex_coord0	: TEXCOORD0,
					uniform sampler2D scene_sampler,
					uniform sampler2D ascii_sampler,
					uniform sampler1D lums_sampler,
					uniform float3 arg) : COLOR
{
	const float3 l_weight = float3(0.299, 0.587, 0.114);

	float lum = dot(tex2D(scene_sampler, tex_coord0), l_weight);
	
	float ascii = round(tex1D(lums_sampler, lum).r * 256);
	float level = round(lum * 8) / 8;
	
	float2 t;
	t.y = floor(ascii / arg.z);
	t.x = ascii - t.y * arg.z;
	t = (t + frac(tex_coord0 / arg.xy)) / float2(arg.z, 1);

	return float4(level * tex2D(ascii_sampler, t).rgb, 1);
}

technique ShowAscii
{
	pass p0
	{
		VertexShader = compile vs_1_1 DownsampleVS();
		PixelShader = compile ps_2_0 ShowAsciiPS(scene_sampler, ascii_sampler, lums_sampler, float3(cell_per_row, cell_per_line, char_per_row));
	}
}
