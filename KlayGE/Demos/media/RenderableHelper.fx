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