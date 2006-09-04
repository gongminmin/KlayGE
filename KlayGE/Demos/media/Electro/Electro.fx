float y;
float z;

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

VS_OUTPUT ElectroVS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.pos = input.pos;
	output.texcoord0 = input.texcoord0 + float3(0, y, z);

	return output;
}


sampler3D electroSampler;

half4 ElectroPS(float3 texCoord0	: TEXCOORD0,
					uniform sampler3D electroMap) : COLOR
{
	half turb = tex3D(electroMap, texCoord0).r * 2 - 1;
	half4 grow = (1 - pow(turb, 0.2)) * half4(1.70, 1.48, 1.78, 1);
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
