float4x4 modelviewporj : WORLDVIEWPROJECTION;
float4x4 modelIT;
float3 light = float3(-1, 0, -1);

struct VS_IN
{
	float4 pos	: POSITION;
	float3 normal : NORMAL;
	float2 tex0	: TEXCOORD0;
};

struct VS_OUT
{
	float4 pos	: POSITION;
	float2 tex0 : TEXCOORD0;
	float4 clr	: COLOR0;
};

VS_OUT MeshVS(VS_IN input)
{
	VS_OUT output;
	
	output.pos = mul(input.pos, modelviewporj);
	output.tex0 = input.tex0;
	
	float3 normal = mul(input.normal, (float3x3)modelIT);
	output.clr = dot(normal, light - input.pos.xyz);

	return output;
}

texture tex;

sampler2D texSampler = sampler_state
{
	Texture = <tex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
};

float4 MeshPS(float4 clr : COLOR0, float2 uv : TEXCOORD0,
				uniform sampler2D texSampler) : COLOR
{
	return clr * tex2D(texSampler, uv);
}

technique KMeshTec
{
	pass p0
	{
		Lighting = false;
		SpecularEnable = false;

		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 MeshVS();
		PixelShader = compile ps_1_1 MeshPS(texSampler);
	}
}
