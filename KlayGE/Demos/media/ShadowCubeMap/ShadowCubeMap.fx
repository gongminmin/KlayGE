float4x4 WorldViewProj;
float4x4 World;
float4x4 InvLightWorld;

sampler LampSampler;
sampler ShadowMapSampler;


void ShadowMapVS(float4 Position : POSITION,
						out float4 oPos : POSITION,
						out float4 oLightWorldPos : TEXCOORD0)
{
    oPos = mul(Position, WorldViewProj);
    oLightWorldPos = mul(mul(Position, World), InvLightWorld);
}

float4 OutputDepthPS(float4 oLightWorldPos : TEXCOORD0) : COLOR
{
	oLightWorldPos /= oLightWorldPos.w;
	return float4(length(oLightWorldPos.xyz).xxx, 1.0);
}

void MainVS(float4 Position : POSITION,
					float3 Normal   : NORMAL,
					out float4 oPos : POSITION,
					out float4 oLightWorldPos : TEXCOORD0)
{
    oLightWorldPos = mul(mul(Position, World), InvLightWorld);
    oPos = mul(Position, WorldViewProj);
}

float4 MainPS(float4 oLightWorldPos : TEXCOORD0) : COLOR 
{
	oLightWorldPos /= oLightWorldPos.w;

	half shadowLength = texCUBE(ShadowMapSampler, oLightWorldPos.xyz).r + 0.02f;
	bool unshadowed   = (length(oLightWorldPos.xyz) <= shadowLength);
	half3 color;
	if (unshadowed)
	{
		color = texCUBE(LampSampler, oLightWorldPos.xyz).rgb;
	}
	else
	{
		color = 0;
	}
	return float4(color, 1.0);
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
