texture texFont;
int halfWidth;
int halfHeight;
float4 color;

void FontVS(float4 position : POSITION,
			float2 texCoord : TEXCOORD0,

			out float4 oPosition : POSITION,
			out float2 oTexCoord : TEXCOORD0,
			out float4 oColor : COLOR)
{
	oPosition.x = (position.x - halfWidth) / halfWidth;
	oPosition.y = (halfHeight - position.y) / halfHeight;
	oPosition.zw = position.zw;

	oColor = color;
	oTexCoord = texCoord;
}

technique fontTec
{
	pass p0
	{
		Lighting = false;

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		AlphaTestEnable = true;
		AlphaRef = 0x08;
		AlphaFunc = GreaterEqual;

		FillMode = Solid;
		CullMode = CCW;
		StencilEnable = false;
		Clipping = true;
		ClipPlaneEnable = 0;
		VertexBlend = Disable;
		IndexedVertexBlendEnable = false;
		FogEnable = false;
		ColorWriteEnable = RED | GREEN | BLUE | ALPHA;

		Texture[0] = <texFont>;
		TextureTransformFlags[0] = Disable;
		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MipFilter[0] = None;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;

		VertexShader = compile vs_1_1 FontVS();
	}
}
