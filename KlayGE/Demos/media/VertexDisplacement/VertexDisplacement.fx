float4x4 worldviewproj : WORLDVIEWPROJECTION;
float currentAngle;

texture flag;

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

		Texture[0] = <flag>;
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

		VertexShader = compile vs_1_1 VertexDisplacementVS(worldviewproj, currentAngle);
	}
}
