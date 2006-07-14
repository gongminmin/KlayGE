int halfWidth;
int halfHeight;

float4x4 mvp;

void Font2DVS(float4 position : POSITION,
			float4 color : COLOR0,
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

void Font3DVS(float4 position : POSITION,
			float4 color : COLOR0,
			float2 texCoord : TEXCOORD0,

			out float4 oPosition : POSITION,
			out float2 oTexCoord : TEXCOORD0,
			out float4 oColor : COLOR)
{
	oPosition = mul(position, mvp);

	oColor = color;
	oTexCoord = texCoord;
}


sampler texFontSampler;

float4 FontPS(float4 clr : COLOR, float2 texCoord : TEXCOORD0) : COLOR0
{
	float alpha = tex2D(texFontSampler, texCoord).r;
	return clr * alpha;
}

technique Font2DTec
{
	pass p0
	{
		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		FillMode = Solid;
		CullMode = CCW;
		StencilEnable = false;
		Clipping = true;
		ClipPlaneEnable = 0;
		ColorWriteEnable = RED | GREEN | BLUE | ALPHA;

		VertexShader = compile vs_1_1 Font2DVS();
		PixelShader = compile ps_1_1 FontPS();
	}
}

technique Font3DTec
{
	pass p0
	{
		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		FillMode = Solid;
		CullMode = CCW;
		StencilEnable = false;
		Clipping = true;
		ClipPlaneEnable = 0;
		ColorWriteEnable = RED | GREEN | BLUE | ALPHA;

		VertexShader = compile vs_1_1 Font3DVS();
		PixelShader = compile ps_1_1 FontPS();
	}
}

