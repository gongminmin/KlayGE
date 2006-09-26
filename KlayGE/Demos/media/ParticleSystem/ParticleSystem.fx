#include "PointSprite.fx"

float4x4 InvWVP;
float4x4 WVP;
float4x4 View;
float4x4 Proj;
float4x4 InvProj;

float PointRadius;

void PointSpriteVS(float4 pos			: POSITION,
					float2 tex			: TEXCOORD0,
					out float4 oPos		: POSITION,
					out float4 oClr     : COLOR0,
					out float2 oTex		: TEXCOORD0,
					out float4 oPosSS	: TEXCOORD1,
					out float4 oCenterView : TEXCOORD2)
{
	oCenterView = mul(float4(pos.xyz, 1), View);
	float4 view_pos = oCenterView;
	view_pos.xy += (float2(tex.x, 1 - tex.y) * 2 - 1) * PointRadius * oCenterView.w;
	oPos = mul(view_pos, Proj);
	oTex = tex;
	oClr = float4(1, 1, 1, pos.w);
	oPosSS = oPos;
}

float2 offset;
sampler particle_sampler;
sampler depth_sampler;

float4 PointSpritePS(float4 clr    : COLOR0,
						float2 tex : TEXCOORD0,
						float4 PosSS : TEXCOORD1,
						float4 CenterView : TEXCOORD2) : COLOR
{
	CenterView /= CenterView.w;
	PosSS /= PosSS.w;
	float4 view_pos = mul(PosSS, InvProj);
	view_pos /= view_pos.w;
	
	float3 view_dir = normalize(view_pos.xyz);
	float v = dot(CenterView.xyz, view_dir);
	float disc = PointRadius * PointRadius - (dot(CenterView.xyz, CenterView.xyz) - v * v);
	if (disc < 0)
	{
		clip(-1);
		return 0;
	}
	else
	{
		float2 tex_coord = PosSS.xy + offset;
		tex_coord.y = -tex_coord.y;
		tex_coord = tex_coord / 2 + 0.5f;
		float depth = tex2D(depth_sampler, tex_coord).r;
		float4 scene_pos = mul(float4(PosSS.xy, depth, 1), InvProj);
		scene_pos /= scene_pos.w;

		float3 intersect = (v - sqrt(disc)) * view_dir;
		clr *= tex2D(particle_sampler, (intersect.xy - CenterView.xy) / PointRadius / 2 + 0.5);
		clr.a *= saturate((scene_pos.z - intersect.z) * 5);
		return clr;
	}
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
	oPos = mul(mul(Position, View), Proj);
	oNormal = Normal;
	oPosOS = Position.xyz;
}

float4 TerrainPS(float3 pos : TEXCOORD0,
					float3 normal : TEXCOORD1) : COLOR 
{
	return float4(dot(normalize(LightPos - pos), normal) * float3(0.5, 1, 0.5), 1);
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


void DepthPassVS(float4 Position : POSITION,
					out float4 oPos : POSITION,
					out float2 oDepth : TEXCOORD0)
{
	oPos = mul(mul(Position, View), Proj);
	oDepth = oPos.zw;
}

float4 DepthPassPS(float2 depth : TEXCOORD0) : COLOR 
{
	return depth.x / depth.y;
}

technique DepthPass
{
	pass p0
	{
		CullMode = CCW;
		FillMode = Solid;

		VertexShader = compile vs_2_0 DepthPassVS();
		PixelShader = compile ps_2_0 DepthPassPS();
	}
}
