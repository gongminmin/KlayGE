float4x4 worldviewproj : WORLDVIEWPROJECTION;
float currentAngle;

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

VS_OUTPUT VertexDisplacementVS(VS_INPUT input,
					uniform float4x4 worldviewproj,
					uniform float currentAngle)
{
	VS_OUTPUT output;

	float4 v = input.pos;
	v.z = sin(input.pos.x + currentAngle);
	v.z += cos(input.pos.y + currentAngle);
	v.z *= (input.pos.x + 2) * 0.2f;

	output.pos = mul(v, worldviewproj);
	output.texcoord0 = input.texcoord0;

	return output;
}

texture flag;

sampler2D flagSampler = sampler_state
{
	Texture = <flag>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

float4 VertexDisplacementPS(float2 texCoord : TEXCOORD0,
		uniform sampler2D flagSampler) : COLOR0
{
	return tex2D(flagSampler, texCoord);
}

technique VertexDisplacement
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 VertexDisplacementVS(worldviewproj, currentAngle);
		PixelShader = compile ps_1_1 VertexDisplacementPS(flagSampler);
	}
}
