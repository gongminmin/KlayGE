technique tec0
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