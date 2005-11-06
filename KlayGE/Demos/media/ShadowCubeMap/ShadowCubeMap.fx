float4x4 WorldViewProj;
float4x4 World;
float4x4 InvLightWorld;
float3 light_pos;

sampler LampSampler;
sampler ShadowMapSampler;


void ShadowMapVS(float4 Position : POSITION,
						out float4 oPos : POSITION,
						out float4 oLightWorldPos : TEXCOORD0)
{
    oPos = mul(Position, WorldViewProj);
    oLightWorldPos = mul(mul(Position, World), InvLightWorld);
}

float4 OutputDepthPS(float3 oLightWorldPos : TEXCOORD0) : COLOR
{
	return float4(dot(oLightWorldPos.xyz, oLightWorldPos.xyz).xxx, 1.0);
}

void MainVS(float4 Position : POSITION,
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

float4 MainPS(half3 diffuse : COLOR0,
				half3 LightWorldPos : TEXCOORD0) : COLOR 
{
	half PixelLengthSq = dot(LightWorldPos, LightWorldPos);
	half ShadowLengthSq = texCUBE(ShadowMapSampler, LightWorldPos).r + 0.03f;
	
	half3 color;
	if (PixelLengthSq <= ShadowLengthSq)
	{
		color = texCUBE(LampSampler, LightWorldPos.xyz).rgb / (0.4 * ShadowLengthSq);
	}
	else
	{
		color = 0;
	}

	return float4(diffuse * color, 1.0);
}

technique GenShadowMap
{
	Pass P0
	{
		VertexShader = compile vs_1_1 ShadowMapVS();
		PixelShader = compile ps_2_0 OutputDepthPS();
	}
}

technique RenderScene
{
	Pass P0
	{
		VertexShader = compile vs_1_1 MainVS();
		PixelShader = compile ps_2_0 MainPS();
	}
}
