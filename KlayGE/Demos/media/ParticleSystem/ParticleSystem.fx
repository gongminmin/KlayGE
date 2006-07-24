#include "PointSprite.fx"

float4x4 View;
float4x4 Proj;
int ViewportWidth;
int ViewportHeight;
float PointSize;
float3 DistanceAttenuation;
float2 PointMinMax;

void PointSpriteVS(float4 pos			: POSITION,
					float2 tex			: TEXCOORD0,
					out float4 oPos		: POSITION,
					out float2 oTex		: TEXCOORD0,
					out float4 oClr     : COLOR0)
{
	oPos = PointSpriteVertexExtend(float4(pos.xyz, 1), float2(tex.x, -tex.y), PointSize, View, Proj,
			float2(ViewportWidth, ViewportHeight), DistanceAttenuation, PointMinMax);
	oTex = tex;
	oClr = float4(1, 1, 1, pos.w);
}

sampler particle_sampler;

float4 PointSpritePS(float2 tex : TEXCOORD0,
						float4 clr    : COLOR0) : COLOR
{
	clr *= tex2D(particle_sampler, tex);
	return clr;
}

technique PointSprite
{
	pass p0
	{
		CullMode = CCW;
		
		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		VertexShader = compile vs_2_0 PointSpriteVS();
		PixelShader = compile ps_2_0 PointSpritePS();
	}
}


float3 LightPos = float3(0, 10, 3);

void TerrainVS(float4 Position : POSITION,
					float3 Normal : NORMAL,
					out float4 oPos : POSITION,
					out float3 oPosOS : TEXCOORD0,
					out float3 oNormal : TEXCOORD1)
{
	oPos = mul(Position, mul(View, Proj));
	oNormal = Normal;
	oPosOS = Position.xyz;
}

float4 TerrainPS(float3 pos : TEXCOORD0, float3 normal : TEXCOORD1) : COLOR 
{
	return dot(normalize(LightPos - pos), normal) * float4(0.5, 1, 0.5, 1);
}

technique Terrain
{
	pass p0
	{
		CullMode = CCW;
		FillMode = Solid;

		VertexShader = compile vs_2_0 TerrainVS();
		PixelShader = compile ps_2_0 TerrainPS();
	}
}
