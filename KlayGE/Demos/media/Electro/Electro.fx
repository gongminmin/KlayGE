float y;
float z;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float3 texcoord0	: TEXCOORD0;
	float2 texcoord1	: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float3 texcoord0	: TEXCOORD0;
	float2 texcoord1	: TEXCOORD1;
};

VS_OUTPUT ElectroVS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.pos = input.pos;
	output.texcoord0 = input.texcoord0 + float3(0, y, z);
	output.texcoord1 = input.texcoord1;

	return output;
}


texture electroMap;

sampler3D electroSampler = sampler_state
{
	Texture = <electroMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Wrap;
	AddressV  = Wrap;
	AddressW  = Wrap;
};

float4 ElectroPS(float3 texCoord0	: TEXCOORD0,
				float2 texCoord1	: TEXCOORD1,

					uniform sampler3D electroMap) : COLOR
{
	float turb = tex3D(electroMap, texCoord0) * 2 - 1;
	float t = saturate(texCoord1.y * -texCoord1.y + 0.15);
	float y = abs(turb * -t + texCoord1.x);

	float4 grow = (1 - pow(y, 0.2)) * float4(1.70, 1.48, 1.78, 0);
	return pow(grow, 4);
}

technique Electro
{
	pass p0
	{
		VertexShader = compile vs_1_1 ElectroVS();
		PixelShader = compile ps_2_0 ElectroPS(electroSampler);
	}
}
