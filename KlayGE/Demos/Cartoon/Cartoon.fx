texture cartoonTex;

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
	}
}
