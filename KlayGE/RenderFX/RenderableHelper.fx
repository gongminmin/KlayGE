float4x4 matViewProj;
float4 color;

void HelperVS(float4 position : POSITION,
			out float4 oPosition : POSITION)
{
	oPosition = mul(position, matViewProj);
}

float4 HelperPS() : COLOR0
{
	return color;
}

technique PointTec
{
	pass p0
	{
		Lighting = false;
		SpecularEnable = false;

		FillMode = Point;
		CullMode = None;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;
		
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = Disable;

		VertexShader = compile vs_1_1 HelperVS();
		PixelShader = compile ps_1_1 HelperPS();
	}
}

technique LineTec
{
	pass p0
	{
		Lighting = false;
		SpecularEnable = false;

		FillMode = WireFrame;
		CullMode = None;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;
		
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = Disable;

		VertexShader = compile vs_1_1 HelperVS();
		PixelShader = compile ps_1_1 HelperPS();
	}
}

technique TriangleTec
{
	pass p0
	{
		Lighting = false;
		SpecularEnable = false;

		FillMode = WireFrame;
		CullMode = None;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;
		
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = Disable;

		VertexShader = compile vs_1_1 HelperVS();
		PixelShader = compile ps_1_1 HelperPS();
	}
}

technique BoxTec
{
	pass p0
	{
		Lighting = false;
		SpecularEnable = false;

		FillMode = WireFrame;
		CullMode = None;
		Stencilenable = false;
		Clipping = true;

		ZEnable      = true;
		ZWriteEnable = true;
		
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = Disable;

		VertexShader = compile vs_1_1 HelperVS();
		PixelShader = compile ps_1_1 HelperPS();
	}
}


float4x4 inv_mvp;

void SkyBoxVS(float4 pos : POSITION,
			out float4 oPos : POSITION,
			out float3 texcoord0 : TEXCOORD0)
{
	oPos = pos;
	texcoord0 = mul(pos, inv_mvp);
}


samplerCUBE skybox_cubeMapSampler;

float4 SkyBoxPS(float3 texCoord0 : TEXCOORD0,
		uniform samplerCUBE skybox_cubeMap) : COLOR
{
	return texCUBE(skybox_cubeMap, texCoord0);
}

technique SkyBoxTec
{
	pass p0
	{
		CullMode = None;
		VertexShader = compile vs_1_1 SkyBoxVS();
		PixelShader = compile ps_1_1 SkyBoxPS(skybox_cubeMapSampler);
	}
}
