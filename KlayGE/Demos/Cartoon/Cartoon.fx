texture cartoonTex;

float4x4 proj : PROJECTION;
float4x4 worldview : WORLDVIEW;

float3 lightDir = normalize(float3(1, -1, 1));

// 把位置从局部坐标系转化为全局坐标系
float4 TransformPosition(float4 pos, float4x4 worldview, float4x4 proj)
{
	return mul(pos, mul(worldview, proj));
}

// 把法线从局部坐标系转化为全局坐标系
float3 TransformNormal(float4 normal, float4x4 worldview)
{
	return normalize(mul(normal.xyz, (float3x3)worldview));
}

// 卡通渲染
float CartoonRendering(float4 vertexPos, float4 vertexNormal, float3 lightDir, float4x4 worldview)
{
	float3 L = normalize(-lightDir - vertexPos);
	float3 N = TransformNormal(vertexNormal, worldview);

	return dot(L, N);
}

struct VS_INPUT
{
	float4 pos			: POSITION;
	float4 normal		: NORMAL;
};

struct VS_OUTPUT
{
	float4 pos			: POSITION;
	float1 texcoord0	: TEXCOORD0;
};

VS_OUTPUT ToonVS(VS_INPUT input)
{
	float4 pos = TransformPosition(input.pos, worldview, proj);

	VS_OUTPUT output;

	output.pos = pos;
	output.texcoord0.x = CartoonRendering(pos, input.normal, lightDir, worldview);

	return output;
}

technique cartoonTec
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

		Texture[0] = <cartoonTex>;
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0] = Disable;
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MipFilter[0] = Point;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;

		AddressU[0] = Clamp;
		AddressV[0] = Clamp;

		VertexShader = compile vs_1_1 ToonVS();
	}
}
