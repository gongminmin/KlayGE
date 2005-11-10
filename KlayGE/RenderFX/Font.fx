int halfWidth;
int halfHeight;

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

sampler2D texFontSampler;

float4 FontPS(float4 clr : COLOR, float2 texCoord : TEXCOORD0,
		uniform sampler2D texFont) : COLOR0
{
	float4 texel = tex2D(texFont, texCoord);
	return clr * texel;
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
		PixelShader = compile ps_1_1 FontPS(texFontSampler);
	}
}
