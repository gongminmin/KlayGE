float4x4 WorldViewProj;
float4x4 World;
float4x4 InvLightWorld;
float3 light_pos;

sampler LampSampler;
sampler ShadowMapSampler;


void GenShadowMapVS(float4 Position : POSITION,
						out float4 oPos : POSITION,
						out float4 oLightWorldPos : TEXCOORD0)
{
    oPos = mul(Position, WorldViewProj);
    oLightWorldPos = mul(mul(Position, World), InvLightWorld);
}

float4 GenShadowMapPS(float3 oLightWorldPos : TEXCOORD0) : COLOR
{
	float dist = length(oLightWorldPos);
	return float4(dist, dist * dist, 0, 1);
}

void RenderSceneVS(float4 Position : POSITION,
					float3 Normal   : NORMAL,
					out float4 oPos : POSITION,
					out float3 oDiffuse : COLOR0,
					out float4 oLightWorldPos : TEXCOORD0)
{
	float4 world_pos = mul(Position, World);
	float3 world_normal = normalize(mul(Normal, (float3x3)World));

	oPos = mul(Position, WorldViewProj);
	oLightWorldPos = mul(world_pos, InvLightWorld);
    oDiffuse = dot(normalize(light_pos - world_pos.xyz), world_normal);
}

float4 RenderScenePS(half3 diffuse : COLOR0,
				float3 LightWorldPos : TEXCOORD0) : COLOR 
{
	float2 moments = texCUBE(ShadowMapSampler, LightWorldPos);
	
	float dist = length(LightWorldPos);
	float3 color = diffuse * texCUBE(LampSampler, LightWorldPos).rgb / (0.4 * moments.y);
	if (dist > moments.x + 0.1f)
	{
		// Variance shadow mapping
		float variance = moments.y - moments.x * moments.x;
		float m_d = moments.x - dist;
		float p_max = variance / (variance + m_d * m_d);

		color *= p_max;
	}

	return float4(color, 1.0);
}

technique GenShadowMap
{
	Pass P0
	{
		CullMode = None;
		
		VertexShader = compile vs_1_1 GenShadowMapVS();
		PixelShader = compile ps_2_0 GenShadowMapPS();
	}
}

technique RenderScene
{
	Pass P0
	{
		CullMode = CCW;
		
		VertexShader = compile vs_1_1 RenderSceneVS();
		PixelShader = compile ps_2_0 RenderScenePS();
	}
}
