float4x4 InvWVP;
float4x4 WVP;
float4x4 View;
float4x4 Proj;
float4x4 InvProj;

float PointRadius;

float3 up_left, up_right, down_left, down_right;

void ParticleVS(float4 pos			: POSITION,
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
sampler scene_sampler;

float4 ParticlePS(float4 clr    : COLOR0,
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

		float depth = tex2D(scene_sampler, tex_coord).a;
		float dir = lerp(lerp(up_left.z, up_right.z, tex_coord.x),
							lerp(down_left.z, down_right.z, tex_coord.x),
							tex_coord.y);

		float3 intersect = (v - sqrt(disc)) * view_dir;
		clr *= tex2D(particle_sampler, (intersect.xy - CenterView.xy) / PointRadius / 2 + 0.5);
		clr.a *= saturate((dir * depth - intersect.z) * 5);
		clip(clr.a - 0.03f);

		return clr;
	}
}

technique Particle
{
	pass p0
	{
		CullMode = CCW;

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		VertexShader = compile vs_2_0 ParticleVS();
		PixelShader = compile ps_2_0 ParticlePS();
	}
}


float3 LightPos = float3(0, 10, 3);

void TerrainVS(float4 Position : POSITION,
					float3 Normal : NORMAL,
					out float4 oPos : POSITION,
					out float3 oPosOS : TEXCOORD0,
					out float3 oNormal : TEXCOORD1,
					out float2 oDepth : TEXCOORD2)
{
	float4 pos_es = mul(Position, View);
	oPos = mul(pos_es, Proj);
	oNormal = Normal;
	oPosOS = Position.xyz;
	oDepth = pos_es.zw;
}

float4 TerrainPS(float3 pos : TEXCOORD0,
					float3 normal : TEXCOORD1,
					float2 depth : TEXCOORD2) : COLOR 
{
	return float4(dot(normalize(LightPos - pos), normal) * float3(0.5, 1, 0.5),
				(depth.x / depth.y - 0.01f) / 100);
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


void CopyVS(float4 pos : POSITION,
					float2 tex : TEXCOORD0,
					out float4 oPos : POSITION,
					out float2 oTex : TEXCOORD0)
{
	oPos = pos;
	oTex = tex;
}

sampler src_sampler;

void CopyPS(float2 tex_coord0 : TEXCOORD0,
				out float4 clr : COLOR0,
				out float depth : DEPTH)
{
	float4 s = tex2D(src_sampler, tex_coord0);
	clr = float4(s.rgb, 1);
	
	float3 dir = lerp(lerp(up_left, up_right, tex_coord0.x),
							lerp(down_left, down_right, tex_coord0.x),
							tex_coord0.y);
	float4 pos_ps = mul(float4(dir * s.a, 1), Proj);
	depth = pos_ps.z / pos_ps.w;
}

technique Copy
{
	pass p0
	{
		CullMode = CCW;
		FillMode = Solid;

		VertexShader = compile vs_2_0 CopyVS();
		PixelShader = compile ps_2_0 CopyPS();
	}
}