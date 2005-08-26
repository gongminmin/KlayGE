float4x4 viewproj : VIEWPROJECTION;
float4x4 world : WORLD;
float4x4 worldviewIT : WORLDVIEWIT;
float4 lightPos;
float4 eyePos;

struct VS_INPUT
{
	float3 pos			: POSITION;
	float3 normal		: NORMAL;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texcoord0	: TEXCOORD0;
	float2 texcoord1	: TEXCOORD1;
};

VS_OUTPUT ToonVS(VS_INPUT input)
{
	float3 pos = mul(input.pos, world);
	float3 normal = normalize(mul(input.normal, (float3x3)worldviewIT));

	float3 L = normalize(lightPos.xyz - pos);
	float3 V = normalize(eyePos.xyz - pos);

	VS_OUTPUT output;
	output.pos = mul(float4(pos, 1), viewproj);
	output.texcoord0 = float2(dot(normal, L), 0);
	output.texcoord1 = float2(dot(normal, V), 0);

	return output;
}

sampler1D toonMapSampler;
sampler1D edgeMapSampler;

float4 ToonPS(float2 toonTex	: TEXCOORD0,
				float2 edgeTex	: TEXCOORD1,

				uniform sampler1D toonMap,
				uniform sampler1D edgeMap) : COLOR
{
	float3 toon = tex1D(toonMap, toonTex.x);
	float3 edge = tex1D(edgeMap, edgeTex.x);

	return float4(toon * edge, 1);
}

technique cartoonTec
{
	pass p0
	{
		FillMode = Solid;
		CullMode = CCW;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;

		VertexShader = compile vs_1_1 ToonVS();
		PixelShader = compile ps_1_1 ToonPS(toonMapSampler, edgeMapSampler);
	}
}
