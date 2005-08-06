float4x4 modelviewproj : WORLDVIEWPROJECTION;
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
	
	output.pos = mul(input.pos, modelviewproj);
	output.tex0 = input.tex0;
	
	float3 normal = mul(input.normal, (float3x3)modelIT);
	output.clr = dot(normal, light - input.pos.xyz);

	return output;
}

sampler2D texSampler;

float4 MeshPS(float4 clr : COLOR0, float2 uv : TEXCOORD0,
				uniform sampler2D tex) : COLOR
{
	return clr * tex2D(tex, uv);
}

float4 MeshNoTexPS(float4 clr : COLOR0) : COLOR
{
	return clr;
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

technique KMeshNoTexTec
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
		PixelShader = compile ps_1_1 MeshNoTexPS();
	}
}

