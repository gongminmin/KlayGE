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


texture skybox_cubemap;

samplerCUBE skybox_cubeMapSampler = sampler_state
{
	Texture = <skybox_cubemap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Clamp;
	AddressV  = Clamp;
	AddressW  = Clamp;
};

float4 SkyBoxPS(float3 texCoord0 : TEXCOORD0) : COLOR
{
	return texCUBE(skybox_cubeMapSampler, texCoord0);
}

technique SkyBoxTec
{
	pass p0
	{
		CullMode = None;
		VertexShader = compile vs_1_1 SkyBoxVS();
		PixelShader = compile ps_1_1 SkyBoxPS();
	}
}
